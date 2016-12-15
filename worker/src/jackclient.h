#pragma once

#include "processor.h"
#include <jackaudioio.hpp>

class JackClient : public JackCpp::AudioIO
{
  public:
    JackClient(Processor* processor);

    virtual int audioCallback(jack_nframes_t nframes, audioBufVector inBufs, audioBufVector outBufs) override;

  private:
    Processor* processor;
};
