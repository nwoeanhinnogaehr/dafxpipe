from numpy import *
from stft import *
from pvoc import *

stft = STFT(2048, 2, 4)
pvoc = PhaseVocoder(stft)

time = 0
def process(i, o):
    global time
    for x in stft.forward(i):
        x = pvoc.forward(x)
        idx = indices(x.shape)
        x = pvoc.shift(x, lambda y: y + 2*sin(idx[1]*0.1+time*0.01))
        x = pvoc.backward(x)
        stft.backward(x)
    stft.pop(o)
    time += 1
