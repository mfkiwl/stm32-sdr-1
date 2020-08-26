#!/usr/bin/env python3

import numpy as np
import matplotlib.pyplot as plt
from scipy import signal
import struct

fs = 48e3

def float2q31(f):
    val = int(f*0x80000000)
    if val > 0x7fffffff:
        val = 0x7fffffff
    elif val < -0x80000000:
        val = -0x80000000

    return struct.unpack(">L", struct.pack(">l",val))[0]

def q312float(q):
    return float(struct.unpack(">l", struct.pack(">L",q))[0]/0x80000000)


def b2cmsis(b):
    num_taps = len(b)
    print(f"const uint8_t numTaps = {num_taps};")
    print(f"const q31_t FilterCoeffs[{num_taps}]","= {",sep=" ")
    for coefficient in reversed(b):
            c = float2q31(coefficient)
            print(f"{c:#010x}, ",end="")
    print("};")
    print(f"q31_t FilterState[{num_taps}+I2S_BUFSIZE2];")

if __name__ == "__main__":
    b = signal.firwin(40, 1.35e3, window='blackman', pass_zero=True, fs=48e3)
    b2cmsis(b)
    plt.subplot(2,1,1)
    plt.plot(b)
    plt.grid()
    
    plt.subplot(2,1,2)
    [w, h] = signal.freqz(b,1, fs=fs)
    plt.semilogx(w,20*np.log10(np.abs(h)))
    plt.grid()
    plt.show()
