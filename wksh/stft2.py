from numpy import *
from stft import *
from pvoc import *

stft = STFT(4096, 2, 2)

prev = None
def process(i, o):
    global prev
    for x in stft.forward(i):
        idx = indices(x.shape)
        x = to_polar(x)
        if prev is None:
            prev = x
        tmp = copy(x)
        x.real = prev.real
        k = 0.99
        prev = x*k + tmp*(1-k)
        x = from_polar(x)
        stft.backward(x)
    stft.pop(o)
