#include "PythonProcessor.h"
#include "debug.h"
#include <iostream>

PythonProcessor::PythonProcessor()
{
    try {
        Py_Initialize();
        np::initialize();

        main_module = py::import("__main__");
        main_namespace = main_module.attr("__dict__");

    } catch (py::error_already_set const&) {
        std::cerr << "Python init error:" << std::endl;
        PyErr_Print();
    }
}

void
PythonProcessor::exec(std::string code)
{
    try {
        py::exec(code.c_str(), main_namespace);
        if (!((py::dict)main_namespace).has_key("process")) {
            DBG_PRINT("No process function defined!");
        }
    } catch (py::error_already_set const&) {
        std::cerr << "Python init error:" << std::endl;
        PyErr_Print();
    }
}

void
PythonProcessor::process(int numInChannels, int numOutChannels, int frameSize, float** inBufs,
                         float** outBufs)
{
    if (!((py::dict)main_namespace).has_key("process"))
        return;

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
    for (int i = 0; i < numInChannels; i++)
        std::copy(outData + i * frameSize, outData + (i + 1) * frameSize, outBufs[i]);
    delete[] inData;
    delete[] outData;
}
