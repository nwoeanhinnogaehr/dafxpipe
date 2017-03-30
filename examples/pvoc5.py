from numpy import *
from stft import *
from pvoc import *

stft = STFT(16384, 2, 4)
pvoc = PhaseVocoder(stft)

time = 0
def process(i, o):
    global time
    for x in stft.forward(i):
        x = pvoc.forward(x)
        x = pvoc.to_bin_offset(x)
        x = pvoc.shift(x, lambda y: sin(y + time*0.01)*mean(y))
        x = pvoc.from_bin_offset(x)
        x = pvoc.backward(x)
        stft.backward(x)
    stft.pop(o)
    time += 1
