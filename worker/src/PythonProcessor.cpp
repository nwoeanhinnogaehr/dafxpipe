#include "PythonProcessor.h"
#include "debug.h"
#include <iostream>

struct AcquireGIL
{
    AcquireGIL()
    {
        if (!PyGILState_Check()) {
            tstate = PyEval_SaveThread();
            gstate = PyGILState_Ensure();
        }
    }
    ~AcquireGIL()
    {
        if (gstate && tstate) {
            PyGILState_Release(gstate);
            PyEval_RestoreThread(tstate);
        }
    }

  private:
    PyThreadState* tstate;
    PyGILState_STATE gstate;
};

PythonProcessor::PythonProcessor(int port, char* argv0)
  : port(port)
{
    try {
        // set PYTHONPATH for convenience
        char cwd[1024];
        getcwd(cwd, sizeof(cwd));
        std::string pythonpath(cwd);
        pythonpath += "/lib/";
        DBG_PRINT("PYTHONPATH=" << pythonpath);
        setenv("PYTHONPATH", pythonpath.c_str(), true);

        // Some libs require that this is set
        wchar_t* programName = Py_DecodeLocale(argv0, NULL);
        Py_SetProgramName(programName);

        Py_Initialize();
        np::initialize();
        PyEval_InitThreads();

        wchar_t* argv[1] { programName };
        PySys_SetArgv(1, argv);

        main_module = py::import("__main__");
        main_namespace = main_module.attr("__dict__");

    } catch (py::error_already_set const&) {
        std::cerr << "Python init error:" << std::endl;
        PyErr_Print();
    }
}

void
PythonProcessor::setupAPI()
{
    std::lock_guard<std::mutex> lock(python_mutex);
    AcquireGIL gil;
    try {
        py::object worker_module = py::import("worker");
        py::object worker_namespace = main_module.attr("__dict__");
        worker_module.attr("__worker_port") = port;
        worker_module.attr("init")();
        main_namespace["worker"] = worker_module;
    } catch (py::error_already_set const&) {
        std::cerr << "Python API init error:" << std::endl;
        PyErr_Print();
    }
}

void
PythonProcessor::exec(std::string code)
{
    std::lock_guard<std::mutex> lock(python_mutex);
    AcquireGIL gil;
    try {
        py::exec(code.c_str(), main_namespace);
        if (!((py::dict)main_namespace).has_key("process"))
            DBG_PRINT("No process function defined!");
    } catch (py::error_already_set const&) {
        std::cerr << "Python exec error:" << std::endl;
        PyErr_Print();
    }
}

void
PythonProcessor::silence()
{
    std::lock_guard<std::mutex> lock(python_mutex);
    AcquireGIL gil;
    if (((py::dict)main_namespace).has_key("process"))
        py::api::delitem(main_namespace, "process");
}

void
PythonProcessor::process(int numInChannels, int numOutChannels, int frameSize, float** inBufs,
                         float** outBufs)
{
    // Clear possibly uninitialized output buffer
    for (int i = 0; i < numOutChannels; i++)
        std::fill(outBufs[i], outBufs[i] + frameSize, 0);

    // Don't process during exec
    if (!python_mutex.try_lock())
        return;

    AcquireGIL gil;

    if (!((py::dict)main_namespace).has_key("process")) {
        python_mutex.unlock();
        return;
    }

    // alloc & copy data in
    float* inData = new float[numInChannels * frameSize];
    float* outData = new float[numOutChannels * frameSize];
    for (int i = 0; i < numInChannels; i++)
        std::copy(inBufs[i], inBufs[i] + frameSize, inData + i * frameSize);

    try {
        // make ndarrays
        np::ndarray pyInput =
          np::from_data(inData, np::dtype::get_builtin<float>(), py::make_tuple(numInChannels, frameSize),
                        py::make_tuple(sizeof(float) * frameSize, sizeof(float)), py::object());
        np::ndarray pyOutput =
          np::from_data(outData, np::dtype::get_builtin<float>(), py::make_tuple(numOutChannels, frameSize),
                        py::make_tuple(sizeof(float) * frameSize, sizeof(float)), py::object());

        // do processing
        py::object processFn = main_namespace["process"];
        processFn(pyInput, pyOutput);
    } catch (py::error_already_set const&) {
        std::cerr << "Python process error:" << std::endl;
        PyErr_Print();
    }

    // copy data out & clean up
    for (int i = 0; i < numOutChannels; i++)
        std::copy(outData + i * frameSize, outData + (i + 1) * frameSize, outBufs[i]);
    delete[] inData;
    delete[] outData;

    python_mutex.unlock();
}
