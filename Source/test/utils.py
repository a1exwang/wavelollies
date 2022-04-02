import numpy as np
import subprocess
import scipy.signal

def give_me_a_periodic_signal(N):
    f = 0.3
    ts = np.arange(N)
    x = scipy.signal.sawtooth(2 * np.pi * f * ts)
    return x

def apply_filter(x, program):
    s = "\n".join(map(lambda v: str(v), x))
    output_bytes = subprocess.check_output(
        [program],
        input=s.encode("utf-8"),
    )
    return np.array(list(map(float, output_bytes.decode("utf-8").strip().split("\n"))), dtype='float32')

def array_diff(output, expected, epsilon = 1e-6):
    diff = np.max(np.abs(output - expected))
    if diff > epsilon:
        print("deviation(%g) exceed %g" % (diff, epsilon))
        print("index  output  expected diff")
        for i, (x, y) in enumerate(zip(output, expected)):
            print("% 6d  %-12g %-12g %-12g" % (i, x, y, np.abs(x-y)))
    else:
        print("passed")