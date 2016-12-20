# super inflexible and basic STFT effect example

from queuebuffer import QueueBuffer
import numpy as np

fftsize = 4096
hop = fftsize // 2

def fx(x):
    # downwards frequency shift
    return np.roll(x, -8, 1)

inq = QueueBuffer((2, fftsize), np.float32, 1)
outq = QueueBuffer((2, fftsize), np.float32, 1)

def process(i, o):
    inq.push(i)
    if inq.size >= fftsize:
        fft_in = inq.read(fftsize)
        inq.pop(hop)
        fft_in *= np.hanning(fftsize)
        spec = np.fft.rfft(fft_in, norm='ortho')
        spec = fx(spec)
        inv = np.fft.irfft(spec, norm='ortho')
        last = outq.pop(hop)
        inv[:,:hop] += last
        outq.push(inv)
    o[:] = outq.pop(o.shape[1])
