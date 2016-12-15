#pragma once

#include "processor.h"
#include <boost/python/numpy.hpp>

namespace py = boost::python;
namespace np = boost::python::numpy;

class PythonProcessor : public Processor
{
  public:
    PythonProcessor();
    virtual void process(int numInChannels, int numOutChannels, int frameSize, float** inBufs,
                         float** outBufs) override;

  private:
    py::object main_module;
    py::object main_namespace;
};
