#include "jackclient.h"

JackClient::JackClient(Processor* processor)
  : JackCpp::AudioIO("jackcpp-test", 2, 2)
  , processor(processor)
{
}

int
JackClient::audioCallback(jack_nframes_t nframes, audioBufVector inBufs, audioBufVector outBufs)
{
    processor->process(inBufs.size(), outBufs.size(), nframes, &inBufs[0], &outBufs[0]);

    // 0 on success
    return 0;
}
