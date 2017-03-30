from numpy import *
from stft import *
from pvoc import *

stft = STFT(1024, 2, 4)
pvoc = PhaseVocoder(stft)

time = 0
def process(i, o):
    global time
    for x in stft.forward(i):
        idx = indices(x.shape)
        x = pvoc.forward(x)
        x = pvoc.shift(x, lambda y: y - (idx[1]+time)%amax(idx[1]))
        x = pvoc.backward(x)
        stft.backward(x)
    stft.pop(o)
    time += 1
