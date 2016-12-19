# you can use the QueueBuffer if you need to do some buffer wrangling
import numpy as np
from queuebuffer import QueueBuffer

delay = 1<<18
f = QueueBuffer((2, delay), np.float32, 1)
b = QueueBuffer((2, delay), np.float32, 1)

def process(i, o):
    f.push(i)
    while f.size >= delay:
        data = f.pop(delay)
        # todo process larger chunk of data here
        b.push(data)
    o[:] = b.pop(o.shape[1])
