import subprocess
import sys
import numpy as np
import scipy.signal

def give_me_a_periodic_signal(N):
    f = 0.01
    ts = np.arange(N)
    x = scipy.signal.sawtooth(2 * np.pi * f * ts)
    return x

def apply_filter(x, program_or_args):
    s = "\n".join(map(lambda v: str(v), x))
    if type(program_or_args) is list:
        args = program_or_args
    else:
        args = [program_or_args]
    output_bytes = subprocess.check_output(
        list(map(str, args)),
        input=s.encode("utf-8"),
    )
    lines = output_bytes.decode("utf-8")
    try:
        ret = np.array(list(map(float, lines.strip().split("\n"))), dtype='float32')
        return ret
    except Exception as e:
        # print(lines)
        print(e)

def array_diff(output, expected, epsilon = 1e-6):
    diff = np.max(np.abs((output - expected)) / expected)
    # stddev = np.sqrt(np.sum((output - expected) ** 2) / len(output))
    print("max deviation = %g, threshold = %g" % (diff, epsilon))
    if diff > epsilon:
        print("index  output  expected diff")
        for i, (x, y) in enumerate(zip(output, expected)):
            print("% 6d  %-14.10f %-14.10f % 8.3fdB" % (i, x, y, 10*np.log10(np.abs(x-y))))
    else:
        print("passed")

def exe(case_name):
    if sys.platform == "win32":
        executable = "./buildwin/Debug/%s.exe" % case_name
    else:
        executable = './build/%s' % case_name
    return executable
        
def format_frequency(f):
    if f < 1000:
        return "%d" % f
    else:
        return "%.1fk" % (f/1000)