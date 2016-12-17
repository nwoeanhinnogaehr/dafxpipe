import numpy as np

class QueueBuffer:
    def __init__(self, shape, dtype, axis=0):
        self.buffer = np.zeros(shape, dtype)
        self.axis = axis
        self.size = 0
    def push(self, x):
        self.buffer = np.roll(self.buffer, -x.shape[self.axis], self.axis)
        slc = [slice(None)] * len(self.buffer.shape)
        slc[self.axis] = slice(self.buffer.shape[self.axis] - x.shape[self.axis], \
                self.buffer.shape[self.axis])
        self.buffer[slc] = x
        self.size += x.shape[self.axis]
        self.size = min(self.size, self.buffer.shape[self.axis])
    def read(self, size):
        slc = [slice(None)] * len(self.buffer.shape)
        slc[self.axis] = slice(self.buffer.shape[self.axis] - self.size,\
                self.buffer.shape[self.axis] - self.size + size)
        pad = [(0, 0)] * len(self.buffer.shape)
        pad[self.axis] = (0, max(0, size - self.size))
        return np.pad(self.buffer[slc], pad, 'constant')
    def erase(self, size):
        self.size -= size
        self.size = max(self.size, 0)
        erase = self.read(size)
        erase[:] = np.zeros(erase.shape)
    def pop(self, size):
        data = np.copy(self.read(size))
        self.erase(size)
        return data
