#include "BufferedProcessor.h"

// run something on the lang thread and (maybe) wait for it to be executed
BufferedProcessor::ExecResult
BufferedProcessor::langThreadExec(std::function<void()> fn)
{
    auto mutex = std::make_shared<std::mutex>();
    auto cv = std::make_shared<std::condition_variable>();
    auto ready = std::make_shared<bool>(false);
    langThread.enqueue([=]() {
        fn();

        std::unique_lock<std::mutex> lk(*mutex);
        *ready = true;
        cv->notify_one();
    });
    return { mutex, cv, ready };
}

BufferedProcessor::BufferedProcessor()
  : inBuffer(1, 0, 1)
  , outBuffer(1, 0, 1)
{
    std::thread([&]() {
        while (true) {
            std::function<void()> fn;
            langThread.wait_dequeue(fn);
            fn();
        }
    }).detach();
}

void
BufferedProcessor::clearBuffer()
{
    Frame garbage;
    while (inBuffer.try_dequeue(garbage))
        ;
    while (outBuffer.try_dequeue(garbage))
        ;
}
