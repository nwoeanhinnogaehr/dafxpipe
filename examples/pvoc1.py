from numpy import *
from stft import *
from pvoc import *

stft = STFT(1024, 2, 4)
pvoc = PhaseVocoder(stft)

def process(i, o):
    for x in stft.forward(i):
        x = pvoc.forward(x)
        noctaves = 1 # shift up one octave
        x = pvoc.shift(x, lambda y: y * (2**noctaves))
        x = pvoc.backward(x)
        stft.backward(x)
    stft.pop(o)
