from numpy import *
from stft import *
from pvoc import *

stft = STFT(256, 2, 2)

def process(i, o):
    for x in stft.forward(i):
        idx = indices(x.shape)
        x = to_polar(x)
        x.imag = -x.imag
        x = from_polar(x)
        stft.backward(x)
    stft.pop(o)
