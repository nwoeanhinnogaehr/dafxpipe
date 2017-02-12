#pragma once

#include "Processor.h"
#include "blockingconcurrentqueue.h"
#include "Frame.h"
#include <condition_variable>
#include <mutex>
#include <vector>

class BufferedProcessor : public Processor
{
  public:
    BufferedProcessor();
    //~BufferedProcessor();
    //virtual void process(size_t numInChannels, size_t numOutChannels,
                         //size_t frameSize, float** inBufs,
                         //float** outBufs) override;
    //virtual void init() override;
    //virtual void exec(std::string code) override;
    //virtual void silence() override;
    virtual void clearBuffer() override;

  protected:
    struct ExecResult
    {
        void wait()
        {
            std::unique_lock<std::mutex> lk(*mutex);
            cv->wait(lk, [&]() { return *ready; });
        }
        std::shared_ptr<std::mutex> mutex;
        std::shared_ptr<std::condition_variable> cv;
        std::shared_ptr<bool> ready;
    };
    ExecResult langThreadExec(std::function<void()> fn);

    // queues for inter-thread communication
    moodycamel::BlockingConcurrentQueue<std::function<void()>> langThread;
    moodycamel::BlockingConcurrentQueue<Frame> inBuffer;
    moodycamel::ConcurrentQueue<Frame> outBuffer;
};
