#pragma once

#include "JackClient.h"
#include "Processor.h"
#include <grpc/grpc.h>
#include <grpc++/server.h>
#include <memory>

class RpcServer
{
  public:
    RpcServer(std::string host, int serverPort, Processor* proc, JackClient* jack);

  private:
    std::unique_ptr<grpc::Server> server;
};
