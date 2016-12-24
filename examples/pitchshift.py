from stft import STFT
from pvoc import PhaseVocoder
import numpy as np

stft = STFT(1024, 2, 4)
pvoc = PhaseVocoder(stft)

def process(input, output):
    for x in stft.forward(input):
        x = pvoc.forward(x)
        x = pvoc.shift(x, lambda y: y * 1.5)
        x = pvoc.backward(x)
        stft.backward(x)
    stft.pop(output)
    output *= 2
