from numpy import *
import scipy.signal as signal
from stft import *
from pvoc import *

time = 0
def process(i, o):
    global time
    time += 1
    # t is the phase array
    t = (indices(o.shape) + time*o.shape[1])[1] / 44100 * 2 * pi
    # some fm synth
    o[:] = signal.sawtooth(t * 50 + signal.sawtooth(t*200, 0.5)+ 4*sin(t*100
        + signal.sawtooth(interp(t/32,linspace(0,7,8),[3,6,7,2,11,4,14,9],period=8)
            + sin(t)*4, 0.5)), 0.5)
