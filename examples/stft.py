import numpy as np
from stft import STFT

stft = STFT(8192, 2, 2)

def process(input, output):
    for spec in stft.forward(input):
        # frequency shift left channel down ~10Hz
        # and right channel up ~10Hz.
        spec[0] = np.roll(spec[0], -2)
        spec[1] = np.roll(spec[1], 2)
        stft.backward(spec)
    stft.pop(output)
