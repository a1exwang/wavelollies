from .utils import apply_filter, array_diff, give_me_a_periodic_signal
import numpy as np

N = 1024
signal = give_me_a_periodic_signal(N)

ground_truth = (np.abs(np.fft.fft(signal)) / N)[:N//2]

output = apply_filter(signal, "build/test_fft")

array_diff(output, ground_truth)