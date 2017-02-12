#pragma once

#include "BufferedProcessor.h"

class JuliaProcessor : public BufferedProcessor
{
  public:
    JuliaProcessor();
    ~JuliaProcessor();
    virtual void process(size_t numInChannels, size_t numOutChannels,
                         size_t frameSize, float** inBufs,
                         float** outBufs) override;
    virtual void init() override;
    virtual void exec(std::string code) override;
    virtual void silence() override;
};
