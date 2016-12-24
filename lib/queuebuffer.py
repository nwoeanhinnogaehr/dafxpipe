import numpy as np

class QueueBuffer:
    def __init__(self, shape, dtype, axis=0):
        self.buffer = np.zeros(shape, dtype)
        self.axis = axis
        self.size = 0


    def sliceFront(self, size):
        slc = [slice(None)] * len(self.buffer.shape)
        slc[self.axis] = slice(self.buffer.shape[self.axis] - self.size, \
                               self.buffer.shape[self.axis] - self.size + size)
        return self.buffer[slc]
    def sliceBack(self, size):
        slc = [slice(None)] * len(self.buffer.shape)
        slc[self.axis] = slice(self.buffer.shape[self.axis] - size, \
                               self.buffer.shape[self.axis])
        return self.buffer[slc]


    def pushBack(self, x):
        x = np.array(x)
        self.buffer = np.roll(self.buffer, -x.shape[self.axis], self.axis)
        self.sliceBack(x.shape[self.axis])[:] = x
        self.size += x.shape[self.axis]
        self.size = min(self.size, self.buffer.shape[self.axis])
    def pushFront(self, x):
        # TODO not yet implemented
        pass


    def peekFront(self, size):
        slc = self.sliceFront(size)
        pad = [(0, 0)] * len(self.buffer.shape)
        pad[self.axis] = (0, max(0, size - self.size))
        return np.pad(slc, pad, 'constant')
    def peekBack(self, size):
        return np.copy(self.sliceBack(size))


    def popFront(self, size):
        data = self.peekFront(size)
        self.eraseFront(size)
        self.size -= size
        self.size = max(self.size, 0)
        return data
    def popBack(self, size):
        data = self.peekBack(size)
        self.eraseBack(size)
        self.size -= size
        self.size = max(self.size, 0)
        self.buffer = np.roll(self.buffer, size, self.axis)
        return data


    def eraseFront(self, size):
        erase = self.sliceFront(size)
        erase[:] = np.zeros(erase.shape)
    def eraseBack(self, size):
        erase = self.sliceBack(size)
        erase[:] = np.zeros(erase.shape)


#TODO finish writing tests
import unittest

class TestQueueBuffer(unittest.TestCase):
    def test_fifo1(self):
        qb = QueueBuffer((16), np.float32)
        qb.pushBack([1,2,3,4])
        self.assertTrue(np.all(qb.peekFront(4) == [1,2,3,4]))
        self.assertTrue(np.all(qb.popFront(4) == [1,2,3,4]))
        self.assertTrue(np.all(qb.popFront(4) == [0,0,0,0]))

    def test_fifo2(self):
        qb = QueueBuffer((16), np.float32)
        qb.pushBack([1,2,3,4])
        qb.pushBack([1,2,3,4])
        qb.pushBack([1,2,3,4,5,6,7,8])
        self.assertTrue(np.all(qb.peekFront(4) == [1,2,3,4]))
        self.assertTrue(np.all(qb.peekFront(4) == [1,2,3,4]))
        self.assertTrue(np.all(qb.popFront(4) == [1,2,3,4]))
        self.assertTrue(np.all(qb.popFront(4) == [1,2,3,4]))
        self.assertTrue(np.all(qb.popFront(8) == [1,2,3,4,5,6,7,8]))
        self.assertTrue(np.all(qb.popFront(4) == [0,0,0,0]))

    def test_fifo3(self):
        qb = QueueBuffer((16), np.float32)
        qb.pushBack([1,2,3,4])
        qb.pushBack([1,2,3,4])
        qb.pushBack([1,2,3,4,5,6,7,8])
        qb.pushBack([1,2,3,4])
        self.assertTrue(np.all(qb.peekFront(4) == [1,2,3,4]))
        self.assertTrue(np.all(qb.popFront(4) == [1,2,3,4]))
        self.assertTrue(np.all(qb.popFront(4) == [1,2,3,4]))
        self.assertTrue(np.all(qb.popFront(4) == [5,6,7,8]))
        self.assertTrue(np.all(qb.popFront(4) == [1,2,3,4]))
        self.assertTrue(np.all(qb.popFront(4) == [0,0,0,0]))
        self.assertTrue(np.all(qb.peekFront(4) == [0,0,0,0]))
        self.assertTrue(np.all(qb.sliceFront(4) == []))

    def test_eraseFront(self):
        qb = QueueBuffer((16), np.float32)
        qb.pushBack([1,2,3,4])
        self.assertTrue(np.all(qb.peekFront(4) == [1,2,3,4]))
        qb.eraseFront(2)
        self.assertTrue(np.all(qb.peekFront(4) == [0,0,3,4]))

if __name__ == '__main__':
    unittest.main()
