#pragma once

#include <string>

class Processor
{
  public:
    virtual void process(int numInChannels, int numOutChannels, int frameSize, float** inBufs,
                         float** outBufs) = 0;
    virtual void init() = 0;
    virtual void exec(std::string code) = 0;
    virtual void silence() = 0;
};
