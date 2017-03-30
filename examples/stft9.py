from numpy import *
from stft import *
from pvoc import *

stft = STFT(1<<14, 2, 4)

def process(i, o):
    for x in stft.forward(i):
        idx = indices(x.shape)
        x = to_polar(x)
        k = 4
        x.imag[:,:-k] = diff(x.imag, k, axis=1)/32
        x = from_polar(x)
        stft.backward(x)
    stft.pop(o)
