#include "JackClient.h"

JackClient::JackClient(Processor* processor, int clientID)
  : JackCpp::AudioIO("worker-" + std::to_string(clientID), 2, 2)
  , processor(processor)
  , clientID(clientID)
{
}

int
JackClient::audioCallback(jack_nframes_t nframes, audioBufVector inBufs, audioBufVector outBufs)
{
    processor->process(inBufs.size(), outBufs.size(), nframes, &inBufs[0], &outBufs[0]);

    // 0 on success
    return 0;
}
