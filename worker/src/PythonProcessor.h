#pragma once

#include "Processor.h"
#include <boost/python/numpy.hpp>
#include <mutex>

namespace py = boost::python;
namespace np = boost::python::numpy;

class PythonProcessor : public Processor
{
  public:
    PythonProcessor();
    virtual void process(int numInChannels, int numOutChannels, int frameSize, float** inBufs,
                         float** outBufs) override;
    void exec(std::string code);

  private:
    py::object main_module;
    py::object main_namespace;
    std::mutex python_mutex;
};
