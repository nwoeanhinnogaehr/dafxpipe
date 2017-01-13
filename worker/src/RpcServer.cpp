#include "RpcServer.h"
#include "debug.h"
#include "worker.grpc.pb.h"
#include <grpc++/security/server_credentials.h>
#include <grpc++/server_builder.h>
#include <grpc++/server_context.h>
#include <string>
#include <thread>

class WorkerImpl : public Worker::Service
{
  public:
    WorkerImpl(Processor* proc, JackClient* jack)
      : proc(proc)
      , jack(jack)
    {
    }

    grpc::Status SetupAPI(grpc::ServerContext*, const Empty*, Empty*) override
    {
        DBG_PRINT("API Init");
        proc->init();
        return grpc::Status::OK;
    }
    grpc::Status Exec(grpc::ServerContext*, const Code* req, Empty*) override
    {
        DBG_PRINT("Exec...");
        proc->exec(req->code());
        DBG_PRINT("OK.");
        return grpc::Status::OK;
    }
    grpc::Status Silence(grpc::ServerContext*, const Empty*, Empty*) override
    {
        DBG_PRINT("Silence");
        proc->silence();
        return grpc::Status::OK;
    }
    grpc::Status SetNumInputs(grpc::ServerContext*, const NumChannels* req, Empty*) override
    {
        DBG_PRINT("Num inputs = " << req->num());
        jack->stop();
        jack->setNumInputs(req->num());
        jack->start();
        return grpc::Status::OK;
    }
    grpc::Status SetNumOutputs(grpc::ServerContext*, const NumChannels* req, Empty*) override
    {
        DBG_PRINT("Num outputs = " << req->num());
        jack->stop();
        jack->setNumOutputs(req->num());
        jack->start();
        return grpc::Status::OK;
    }
    grpc::Status GetNumInputs(grpc::ServerContext*, const Empty*, NumChannels* resp) override
    {
        resp->set_num(jack->inPorts());
        return grpc::Status::OK;
    }
    grpc::Status GetNumOutputs(grpc::ServerContext*, const Empty*, NumChannels* resp) override
    {
        resp->set_num(jack->outPorts());
        return grpc::Status::OK;
    }
    grpc::Status GetSampleRate(grpc::ServerContext*, const Empty*, SampleRate* resp) override
    {
        resp->set_rate(jack->getSampleRate());
        return grpc::Status::OK;
    }
    grpc::Status GetBufferSize(grpc::ServerContext*, const Empty*, BufferSize* resp) override
    {
        resp->set_size(jack->getBufferSize());
        return grpc::Status::OK;
    }

  private:
    Processor* proc;
    JackClient* jack;
};

RpcServer::RpcServer(std::string host, int port, Processor* proc, JackClient* jack)
{
    std::thread([=]() {
        std::string server_address(host + ":" + std::to_string(port));
        WorkerImpl service(proc, jack);
        grpc::ServerBuilder builder;
        builder.AddListeningPort(server_address, grpc::InsecureServerCredentials());
        builder.RegisterService(&service);
        DBG_PRINT("Listening on " << server_address);
        std::unique_ptr<grpc::Server> server(builder.BuildAndStart());
        server->Wait();
    }).detach();
}
