#pragma once

#include "JackClient.h"
#include "PythonProcessor.h"
#include <grpc/grpc.h>
#include <grpc++/server.h>
#include <memory>

class RpcServer
{
  public:
    RpcServer(std::string host, int serverPort, PythonProcessor* proc, JackClient* jack);

  private:
    std::unique_ptr<grpc::Server> server;
};
