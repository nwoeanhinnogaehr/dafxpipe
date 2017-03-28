from numpy import *
from stft import *
from pvoc import *

stft = STFT(65536, 2, 4)

time = 0
def process(i, o):
    global time
    for x in stft.forward(i):
        idx = indices(x.shape)
        x = to_polar(x)
        fact = sin(log1p(idx[1])*32 + time*0.1)*0.5 + 0.5
        x.real *= fact
        x = from_polar(x)
        stft.backward(x)
    stft.pop(o)
    time += 1
