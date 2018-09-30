import numpy as np

time = 0

def process(_, output):
    global time
    t = 1000000-(np.arange(output.shape[1], dtype=np.int32) + time)
    samples = t / (1|t % (1|(t >> 16 ^ t >> 6 ^ t >> 12)))
    output[:] = (samples % 16) / 8 - 1
    time += output.shape[1]
