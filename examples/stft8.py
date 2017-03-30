from numpy import *
from stft import *
from pvoc import *

stft = STFT(1<<9, 2, 4)

def process(i, o):
    for x in stft.forward(i):
        idx = indices(x.shape)
        x = to_polar(x)
        k = 256
        x.real = gradient(x.real, k, axis=1)*k
        x = from_polar(x)
        stft.backward(x)
    stft.pop(o)
