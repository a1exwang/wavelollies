from .utils import apply_filter, array_diff, give_me_a_periodic_signal
import scipy.signal

N = 1024
signal = give_me_a_periodic_signal(N)

window = scipy.signal.windows.blackmanharris(N).astype('float32')
ground_truth = signal * window

output = apply_filter(signal, "build/test_window")

array_diff(output, ground_truth)