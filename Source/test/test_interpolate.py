import numpy as np
from .utils import apply_filter, give_me_a_periodic_signal
import matplotlib.pyplot as plt

N = 4096
bins = 512
sr = 44100
f = 0.01
ts = np.arange(N)
x = np.sin(2 * np.pi * f * ts)

spec = (np.abs(np.fft.fft(x)) / N)[:N//2]

output = apply_filter(spec, ["build/test_interpolate", str(sr), str(bins)])
plt.plot(output)
plt.savefig('data.png')  