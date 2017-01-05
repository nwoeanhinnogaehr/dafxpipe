from stft import STFT
import numpy as np

size = 1024
shift = 255
stft = STFT(256, 2, 2)
buffer = np.zeros((size, stft.channels, stft.size//2+1), dtype=np.complex128)

k = 0
i = 0
def process(input, output):
    global k
    global i
    for x in stft.forward(input):
        buffer[i] = x
        stft.backward(buffer[k])
        k = (k + shift) % size
        i = (i + 1) % size
    stft.pop(output)
