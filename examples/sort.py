import numpy as np
from stft import *
from pvoc import *

stft = STFT(65536, 2, 2)

def process(input, output):
    for x in stft.forward(input):
        x = to_polar(x)
        idx=np.argsort(-x.real)
        x[:].real = x[:,idx[1]].real
        x = from_polar(x)
        stft.backward(x)
    stft.pop(output)
