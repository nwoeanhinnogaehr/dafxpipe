from numpy import *
from stft import *
from pvoc import *

stft = STFT(65536, 2, 2)

def process(i, o):
    for x in stft.forward(i):
        idx = indices(x.shape)
        x = to_polar(x)
        idx = indices(x.shape)
        x.imag += pow(idx[1]*0.001, 8)
        #x.imag *= log1p(idx[1]*0.0005)+1
        x = from_polar(x)
        stft.backward(x)
    stft.pop(o)
