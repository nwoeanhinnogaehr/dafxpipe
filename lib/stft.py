import numpy as np
from queuebuffer import QueueBuffer

class STFT:
    def __init__(self, size, channels, overlap):
        self.size = size
        self.channels = channels
        self.overlap = overlap
        self.hop = self.size // overlap
        bufSize = max(size, 4096 * 2)
        self.inq = QueueBuffer((channels, bufSize), np.float32, 1)
        self.outq = QueueBuffer((channels, bufSize), np.float32, 1)
        self.window = np.sqrt(np.hanning(size))

    def forward(self, samples):
        self.inq.pushBack(samples)
        while self.inq.size >= self.size:
            fft_in = self.inq.peekFront(self.size)
            self.inq.popFront(self.hop)
            fft_in *= self.window
            spec = np.fft.rfft(fft_in, norm='ortho')
            yield spec

    def backward(self, spec):
        inv = np.fft.irfft(spec, norm='ortho')
        inv *= self.window
        last = self.outq.popBack(self.size - self.hop)
        inv[:,:self.size - self.hop] += last
        self.outq.pushBack(inv)

    def pop(self, output):
        output[:] = self.outq.popFront(output.shape[1])
