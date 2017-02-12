#pragma once

#include <vector>

// represents a single buffer of audio from JACK
// usually this is 256 - 4096 samples
struct Frame
{
    Frame()
      : data({})
      , numInChannels(0)
      , numOutChannels(0)
      , size(0)
    {
    }
    Frame(std::vector<float> data, size_t numInChannels, size_t numOutChannels,
          size_t size)
      : data(data)
      , numInChannels(numInChannels)
      , numOutChannels(numOutChannels)
      , size(size)
    {
    }
    std::vector<float> data;
    size_t numInChannels, numOutChannels, size;
};
