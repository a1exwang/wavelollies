from .utils import apply_filter, array_diff, give_me_a_periodic_signal
import numpy as np

N = 1024
epsilon = 1e-5
signal = give_me_a_periodic_signal(N)

ground_truth = (np.abs(np.fft.fft(signal)))[:N//2]

output = apply_filter(signal, "buildwin/Debug/test_fft.exe")

array_diff(output, ground_truth, epsilon)