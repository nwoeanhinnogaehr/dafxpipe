#pragma once

class Processor
{
  public:
    virtual void process(int numInChannels, int numOutChannels, int frameSize,
                         float** inBufs, float** outBufs) = 0;
};
