from numpy import *
from stft import *
from pvoc import *

stft = STFT(1024, 2, 4)
pvoc = PhaseVocoder(stft)

def process(i, o):
    for x in stft.forward(i):
        x = pvoc.forward(x)
        idx = indices(x.shape)
        x = pvoc.shift(x, lambda y: where(sin(idx[1]) > 0, y, y - 100))
        x = pvoc.backward(x)
        stft.backward(x)
    stft.pop(o)
    o *= 0.5
