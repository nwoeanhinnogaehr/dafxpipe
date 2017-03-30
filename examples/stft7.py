from numpy import *
from stft import *
from pvoc import *

stft = STFT(32768, 2, 2)
last = None

def process(i, o):
    global last
    for x in stft.forward(i):
        idx = indices(x.shape)
        x = to_polar(x)
        if last is None:
            last = x
        x.real = where(x.real > mean(x.real), minimum(last.real*1.1, 1), x.real)
        last = x
        x = from_polar(x)
        stft.backward(x)
    stft.pop(o)
