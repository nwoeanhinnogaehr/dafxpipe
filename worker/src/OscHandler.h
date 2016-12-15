#pragma once

#include "PythonProcessor.h"
#include <lo/lo_cpp.h>

class OscHandler
{
  public:
    OscHandler(std::string host, int serverPort, int clientPort, PythonProcessor* proc);
    void start();

  private:
    void registerHandlers();

    PythonProcessor* proc;
    lo::ServerThread server;
    lo::Address client;
};
