import numpy as np
worker.setNumInputs(0)

time = 0
def process(_, output):
    global time
    t = np.arange(output.shape[1], dtype=np.int32) + time
    samples = t / (1 + t % (1 + t >> 9 | t >> 13))
    output[:] = (samples % 256) / 128 - 1
    time += output.shape[1]
