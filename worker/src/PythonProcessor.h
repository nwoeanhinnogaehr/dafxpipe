#pragma once

#include "BufferedProcessor.h"
#include <boost/python/numpy.hpp>
#include <mutex>

namespace py = boost::python;
namespace np = boost::python::numpy;

class PythonProcessor : public BufferedProcessor
{
  public:
    PythonProcessor(int port, char *argv0);
    virtual void process(size_t numInChannels, size_t numOutChannels, size_t frameSize, float** inBufs,
                         float** outBufs) override;
    virtual void init() override;
    virtual void exec(std::string code) override;
    virtual void silence() override;

  private:
    void saveNamespace();
    void restoreNamespace();

    int port;
    py::object mainModule;
    py::object mainNamespace, lastMainNamespace;
    std::mutex pythonMutex;
};
