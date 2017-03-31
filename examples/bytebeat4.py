from numpy import *
from stft import *
from pvoc import *

stft = STFT(1<<12, 2, 2)

time = 0
def process(i, o):
    global time
    for x in stft.forward(i):
        idx = (indices(x.shape)[1] + time).astype(int)
        x.real = sin((1+ idx % (1 +  idx / (1 + idx << 11 | idx << 13))))/(indices(x.shape)[1]+1)*50
        x.imag = sin(idx % (1 + idx / (1 + (idx << 10 | idx << 12))))/(indices(x.shape)[1]+1)*50
        stft.backward(x)
    stft.pop(o)
    time += o.shape[1]
