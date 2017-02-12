#pragma once

#include <string>

class Processor
{
  public:
    virtual void process(size_t numInChannels, size_t numOutChannels, size_t frameSize, float** inBufs,
                         float** outBufs) = 0;
    virtual void init() = 0;
    virtual void exec(std::string code) = 0;
    virtual void silence() = 0;
    virtual void clearBuffer() = 0;
};
