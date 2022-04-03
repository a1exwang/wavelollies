from .utils import apply_filter, array_diff, give_me_a_periodic_signal
import sys
import numpy as np
import matplotlib.pyplot as plt
import scipy.signal

sr = 48000
stride_order = 6
window_order = 11
fft_order = 16
nsinc = 128
bins = 2048
mindb = -96
maxdb = 0

N = 2 ** window_order
f = 128

ts = np.arange(N)
signal = (scipy.signal.sawtooth(2 * np.pi * f * ts / sr) + scipy.signal.sawtooth(2 * np.pi * 440 * ts / sr)) / 2
# signal = np.sin(2 * np.pi * f * ts / sr)

executable = "./buildwin/Debug/test_e2e.exe" if sys.platform == "win32"  else './build/test_e2e'
output = apply_filter(signal, [executable, sr, stride_order, fft_order, bins, nsinc, mindb, maxdb])

# print('\n'.join(map(str, output)))
plt.plot(output[:bins])
plt.savefig("data.png")