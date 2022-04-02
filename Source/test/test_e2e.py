from .utils import apply_filter, array_diff, give_me_a_periodic_signal
import numpy as np
import matplotlib.pyplot as plt

window_order = 11
N = 2 ** window_order
f = 131
sr = 44100

ts = np.arange(N)
signal = np.sin(2 * np.pi * f * ts / sr)

bins = 2048
nsinc = 2
stride_order = 6
fft_order = 16

output = apply_filter(signal, ["build/test_e2e", sr, stride_order, fft_order, bins, nsinc])

plt.plot(output)
plt.savefig("data.png")