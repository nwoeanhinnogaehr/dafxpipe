#include "JackClient.h"

JackClient::JackClient(Processor* processor, int clientID)
  : JackCpp::AudioIO("worker-" + std::to_string(clientID), 0, 0)
  , processor(processor)
  , clientID(clientID)
{
    setNumInputs(2);
    setNumOutputs(2);
}

int
JackClient::audioCallback(jack_nframes_t nframes, audioBufVector inBufs, audioBufVector outBufs)
{
    processor->process(inBufs.size(), outBufs.size(), nframes, &inBufs[0], &outBufs[0]);

    // 0 on success
    return 0;
}

void
JackClient::setNumInputs(unsigned num)
{
    unsigned numBefore = inPorts();
    if (num > numBefore) {
        reserveInPorts(num);
        for (unsigned i = numBefore; i < num; i++)
            addInPort("input" + std::to_string(i));
    }
    if (num < numBefore) {
        for (unsigned i = num; i < numBefore; i++)
            removeInPort("input" + std::to_string(i));
    }
}

void
JackClient::setNumOutputs(unsigned num)
{
    unsigned numBefore = outPorts();
    if (num > numBefore) {
        reserveOutPorts(num);
        for (unsigned i = numBefore; i < num; i++)
            addOutPort("output" + std::to_string(i));
    }
    if (num < numBefore) {
        for (unsigned i = num; i < numBefore; i++)
            removeOutPort("output" + std::to_string(i));
    }
}
