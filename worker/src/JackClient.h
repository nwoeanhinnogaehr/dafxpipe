#pragma once

#include "Processor.h"
#include <jackaudioio.hpp>

class JackClient : public JackCpp::AudioIO
{
  public:
    JackClient(Processor* processor, int clientID);

    virtual int audioCallback(jack_nframes_t nframes, audioBufVector inBufs, audioBufVector outBufs) override;
    void setNumInputs(unsigned num);
    void setNumOutputs(unsigned num);

  private:
    Processor* processor;
    int clientID;
};
