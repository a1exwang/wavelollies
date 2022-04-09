from .utils import apply_filter, array_diff, exe, give_me_a_periodic_signal
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
f = 64

ts = np.arange(N)
# signal = (scipy.signal.sawtooth(2 * np.pi * f * ts / sr) + scipy.signal.sawtooth(2 * np.pi * 440 * ts / sr)) / 2
signal = np.sin(2 * np.pi * f * ts / sr)

output = apply_filter(signal, [exe("test_e2e"), sr, stride_order, window_order, fft_order, bins, nsinc, mindb, maxdb, "true"])

# print('\n'.join(map(str, output)))
print(output)
plt.plot(output[:bins])
plt.savefig("data.png")