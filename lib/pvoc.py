import numpy as np

def to_polar(x):
    """
    Convert from cartesian to polar coordinates, where the magnitude and
    phase are stored as the real and imaginary parts of a complex number,
    respectively.
    """
    return np.abs(x) + 1j * np.angle(x)

def from_polar(x):
    """
    Convert from polar to cartesian coordinates. The polar input is expected
    to have its magnitude and phase stored as the real and imaginary parts
    of a complex number, respectively.
    """
    return x.real * np.exp(1j * x.imag)

class PhaseVocoder:
    """
    The phase vocoder is a transformation that allows for more fine grained
    frequency analysis and manipulation than can be achieved from a basic
    fourier transform. It requires that some state is maintained between
    calls, so a PhaseVocoder object should be preserved globally.
    """
    def __init__(self, stft):
        """
        Create a new phase vocoder with a given STFT object.
        """
        self.shape = (stft.channels, stft.size//2+1)
        self.hop = 1/stft.overlap
        self.last_phase = np.zeros(self.shape)
        self.sum_phase = np.zeros(self.shape)
        self.frame_size = self.shape[1]
        self.freq_per_bin = 44100 / self.frame_size / 2.0
        self.step_size = self.frame_size * self.hop
        self.expect = 2.0 * np.pi * self.hop
        self.bins = np.arange(self.frame_size)

    def forward(self, x):
        """
        Performs a forwards phase vocoder transform on x. x is expected to be
        in cartesian form. An array of complex numbers is returned where the
        real part represents amplitude and the imaginary part represents
        frequency.
        """
        polar = to_polar(x)
        p = polar.imag - self.last_phase
        self.last_phase = polar.imag
        p -= self.bins * self.expect
        qpd = (p / np.pi).astype(int)
        qpd = qpd + (qpd >= 0) * (qpd & 1) - (qpd < 0) * (qpd & 1)
        p -= np.pi * qpd.astype(float)
        p = p / self.hop / (2.0 * np.pi)
        p = self.bins * self.freq_per_bin + p * self.freq_per_bin
        return polar.real + p*1j

    def backward(self, x):
        """
        Performs a backwards phase vocoder transform on x. x is expected to be
        in the form returned by forward(). An array of complex numbers in
        cartesian form is returned.
        """
        p = x.imag - self.bins * self.freq_per_bin
        p /= self.freq_per_bin
        p *= self.expect
        p += self.bins * self.expect
        self.sum_phase += p
        return from_polar(x.real + 1j*self.sum_phase)

    def shift(self, x, fn):
        """
        A shift operation, which can be used to implement various pitch shift
        operations, among many other more exotic effects.
        x is transformed by applying fn to the frequencies in x, then fn is
        applied to the expected center frequencies of the bins in x, to handle
        bin overflow.
        """
        row, col = np.indices(x.shape)

        # transform coordinates
        col = col.astype(float)
        col = fn(col*self.freq_per_bin)/self.freq_per_bin
        col = np.round(col).astype(int)
        col = np.clip(col, 0, x.shape[-1]-1)

        # transform frequencies
        x.imag = fn(x.imag)

        # remap
        y = np.zeros(x.shape, np.complex128)
        y[row, col] += x
        return y

    def to_bin_offset(self, x):
        """
        Converts frequency into bin relative form - the center of each bin
        becomes that bin's zero frequency.
        """
        return x.real + 1j*(x.imag - self.freq_per_bin*np.indices(x.shape)[1] + self.freq_per_bin/2)

    def from_bin_offset(self, x):
        """
        The inverse of to_bin_offset().
        """
        return x.real + 1j*(x.imag + self.freq_per_bin*np.indices(x.shape)[1] - self.freq_per_bin/2)
