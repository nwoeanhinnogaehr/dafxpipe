import numpy as np

last = 0
def process(i, o):
    global last
    x = i
    x = last + np.cumsum(x, axis=1)/32
    last = x[:,-1:]
    o[:] = np.sin(x)
