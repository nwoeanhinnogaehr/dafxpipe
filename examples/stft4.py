from numpy import *
from stft import *
from pvoc import *

stft = STFT(4096, 2, 2)
last = None

def process(i, o):
    global last
    for x in stft.forward(i):
        x = to_polar(x)
        if last is None:
            last = x
        x.real = where(x.real < mean(x.real) + std(x.real), last.real*0.999, x.real)
        last = x
        x = from_polar(x)
        stft.backward(x)
    stft.pop(o)
    o *= 0.5
