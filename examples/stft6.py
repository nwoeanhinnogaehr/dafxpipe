from numpy import *
from stft import *
from pvoc import *

stft = STFT(1<<18, 2, 2)

def process(i, o):
    for x in stft.forward(i):
        idx = indices(x.shape)
        x = to_polar(x)
        # randomize phase
        x.imag = random.sample(x.imag.shape)*2*pi - pi
        x = from_polar(x)
        stft.backward(x)
    stft.pop(o)
