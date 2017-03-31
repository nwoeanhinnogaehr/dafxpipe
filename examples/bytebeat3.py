from numpy import *
from stft import *
from pvoc import *

stft = STFT(1<<11, 2, 2)

time = 0
def process(i, o):
    global time
    for x in stft.forward(i):
        idx = (indices(x.shape)[1] + time).astype(int)
        x = to_polar(x)
        x.real = sin((1+ idx % (1 + (idx << 13 | idx << 8))))/(indices(x.shape)[1]+1)-0.9
        x.imag = idx % (1 + idx / (1 + (idx << 13 | idx << 7)))
        x = from_polar(x)
        stft.backward(x)
    stft.pop(o)
    time += o.shape[1]
