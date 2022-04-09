from .utils import apply_filter, array_diff, exe, format_frequency, give_me_a_periodic_signal
import sys
import numpy as np
import matplotlib.pyplot as plt
import scipy.signal

sr = 44100
stride_order = 0
window_order = 7
fft_order = 12
nsinc = 8
bins = 1200
mindb = -96
maxdb = 0

stride_size = 1 << stride_order
window_size = 1 << window_order

N = 512
f = 128

ts = np.arange(N)
# signal = (scipy.signal.sawtooth(2 * np.pi * f * ts / sr) + scipy.signal.sawtooth(2 * np.pi * 440 * ts / sr)) / 2
# signal = scipy.signal.chirp(ts/sr, 440, 16384/sr, 55, method='logarithmic')
signal = np.sin(2 * np.pi * 260 * ts / sr)

input = []
N / stride_size
start = 0
blocks = 0
while start + window_size <= N:
    window_signal = signal[start:start+window_size]
    input.append(window_signal)
    start += stride_size
    blocks += 1
input = np.concatenate(input)
output = apply_filter(input, [exe("test_e2e"), sr, stride_order, window_order, fft_order, bins, nsinc, mindb, maxdb, "true"])
output = (output - mindb) / (maxdb - mindb)

img = output.reshape((blocks, bins))

with open("test.log", "w") as file:
    for col in img:
        print(' '.join(map(str, col)), file=file)

fs = (31.75, 62.5, 125, 250, 500, 1000, 2000, 4000, 8000, 16000)
f_positions = list(map(lambda f: int(np.log(f/20)/np.log(20000/20) * bins), fs))
f_labels = list(map(format_frequency, fs))
plt.yticks(f_positions, f_labels)
plt.imshow(img.T, origin='lower', interpolation='bilinear', aspect='auto')   
# print('\n'.join(map(str, output)))
# plt.plot(output[:bins])
# plt.plot(ts/sr, signal)
plt.savefig("data.png")