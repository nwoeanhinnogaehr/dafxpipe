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
  : BufferedProcessor()
  , port(port)
{
    // spawn processing thread
    std::thread([&]() {
        Frame frame;

        while (true) {
            // wait for a frame of audio
            inBuffer.wait_dequeue(frame);

            langThreadExec([&]() {
                AcquireGIL gil;
                if (!((py::dict)mainNamespace).has_key("process")) {
                    outBuffer.try_enqueue(Frame(
                      std::vector<float>(frame.size * frame.numOutChannels, 0),
                      frame.numInChannels, frame.numOutChannels, frame.size));
                    return;
                }

                // alloc & copy data in
                float* inData = new float[frame.numInChannels * frame.size];
                float* outData = new float[frame.numOutChannels * frame.size];
                std::copy(frame.data.begin(), frame.data.end(), inData);

                try {
                    // make ndarrays
                    np::ndarray pyInput = np::from_data(
                      inData, np::dtype::get_builtin<float>(),
                      py::make_tuple(frame.numInChannels, frame.size),
                      py::make_tuple(sizeof(float) * frame.size, sizeof(float)),
                      py::object());
                    np::ndarray pyOutput = np::from_data(
                      outData, np::dtype::get_builtin<float>(),
                      py::make_tuple(frame.numOutChannels, frame.size),
                      py::make_tuple(sizeof(float) * frame.size, sizeof(float)),
                      py::object());

                    // do processing
                    py::object processFn = mainNamespace["process"];
                    processFn(pyInput, pyOutput);

                    // save known working config
                    saveNamespace();
                } catch (py::error_already_set const&) {
                    std::cerr << "Python process error:" << std::endl;
                    PyErr_Print();
                    restoreNamespace();
                }

                if (!outBuffer.try_enqueue(Frame(
                      std::vector<float>(
                        outData, outData + frame.size * frame.numOutChannels),
                      frame.numInChannels, frame.numOutChannels, frame.size))) {
                    std::cerr << "WARNING: No space for frame on output queue!"
                              << std::endl;
                }

                // copy data out & clean up
                delete[] inData;
                delete[] outData;
            }).wait();
        }
    }).detach();

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

        wchar_t* argv[1]{ programName };
        PySys_SetArgv(1, argv);

        mainModule = py::import("__main__");
        mainNamespace = mainModule.attr("__dict__");

        saveNamespace();
    } catch (py::error_already_set const&) {
        std::cerr << "Python init error:" << std::endl;
        PyErr_Print();
    }
}

void
PythonProcessor::init()
{
    langThreadExec([&]() {
        AcquireGIL gil;
        try {
            py::object workerModule = py::import("worker");
            workerModule.attr("init")(port);
            mainNamespace["worker"] = workerModule;
        } catch (py::error_already_set const&) {
            std::cerr << "Python API init error:" << std::endl;
            PyErr_Print();
        }
    }).wait();
}

void
PythonProcessor::exec(std::string code)
{
    langThreadExec([&]() {
        AcquireGIL gil;
        try {
            py::exec(code.c_str(), mainNamespace);
            if (!((py::dict)mainNamespace).has_key("process"))
                DBG_PRINT("No process function defined!");
        } catch (py::error_already_set const&) {
            std::cerr << "Python exec error:" << std::endl;
            PyErr_Print();
            restoreNamespace();
        }
    }).wait();
}

void
PythonProcessor::silence()
{
    langThreadExec([&]() {
        AcquireGIL gil;
        if (((py::dict)mainNamespace).has_key("process"))
            py::api::delitem(mainNamespace, "process");
    }).wait();
}

void
PythonProcessor::process(size_t numInChannels, size_t numOutChannels,
                         size_t frameSize, float** inBufs, float** outBufs)
{
    // clear possibly uninitialized output
    for (size_t i = 0; i < numOutChannels; i++)
        std::fill(outBufs[i], outBufs[i] + frameSize, 0);

    // enqueue input data for processing
    std::vector<float> dry(frameSize * numInChannels);
    for (size_t j = 0; j < frameSize; j++)
        for (size_t i = 0; i < numInChannels; i++)
            dry[j + i * frameSize] = inBufs[i][j];
    if (!inBuffer.try_enqueue(
          Frame(dry, numInChannels, numOutChannels, frameSize)))
        std::cerr << "WARNING: No space for frame on input queue!" << std::endl;

    // pop processed output data, if any
    Frame wet;
    if (outBuffer.try_dequeue(wet) && wet.size == frameSize &&
        wet.numInChannels == numInChannels &&
        wet.numOutChannels == numOutChannels) {
        for (size_t j = 0; j < frameSize; j++)
            for (size_t i = 0; i < numOutChannels; i++)
                outBufs[i][j] = wet.data[j + i * frameSize];
    } else
        std::cerr << "Buffer underrun!" << std::endl;
}

void
PythonProcessor::saveNamespace()
{
    static py::object copyFn = py::import("copy").attr("copy");
    lastMainNamespace = copyFn(mainNamespace);
}

void
PythonProcessor::restoreNamespace()
{
    mainNamespace = lastMainNamespace;
}
