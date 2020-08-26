#!/usr/bin/env python3

import numpy as np
import matplotlib.pyplot as plt
from scipy import signal
import struct

fs = 48e3

def ellip(fs, fpass, fstop, gpass, gstop):
    wpass = 2*fpass/fs
    wstop = 2*fstop/fs
    [order, wn] = signal.ellipord(wpass, wstop, gpass, gstop)
    sos = signal.ellip(order, gpass, gstop, [wn], output='sos')
    return sos

def cheby(fs, fpass, fstop, gpass, gstop):
    wpass = 2*fpass/fs
    wstop = 2*fstop/fs
    [order, wn] = signal.cheb1ord(wpass, wstop, gpass, gstop)
    sos = signal.cheby1(order,gpass,[wn], btype='lowpass', output='sos')
    return sos

def ellip_ba(fs, fpass, fstop, gpass, gstop):
    wpass = 2*fpass/fs
    wstop = 2*fstop/fs
    [order, wn] = signal.ellipord(wpass, wstop, gpass, gstop)
    sos = signal.ellip(order, gpass, gstop, [wn], output='ba')
    return sos

def ellip_bp():
    fs = 48e3
    fpass = np.array([300, 4500])
    fstop = np.array([0, 5000])
    gpass = 1
    gstop = 60

    wpass = 2*fpass/fs
    wstop = 2*fstop/fs

    [order, wn] = signal.ellipord(wpass, wstop, gpass, gstop)
    print(wn)
    sos = signal.ellip(order, gpass, gstop, wn, btype='bandpass', output='sos')
    return sos

def float2q31(f):
    val = int(f*0x80000000)
    if val > 0x7fffffff:
        val = 0x7fffffff
    elif val < -0x80000000:
        val = -0x80000000

    return struct.unpack(">L", struct.pack(">l",val))[0]

def q312float(q):
    return float(struct.unpack(">l", struct.pack(">L",q))[0]/0x80000000)


def sos2cmsis(sos):
    num_stages = sos.shape[0]
    print(f"const uint8_t FilterStages = {num_stages};")
    print(f"const q31_t FilterCoeffs[{5*num_stages}]","= {",sep=" ")
    for section in sos:
        print("    ",end="")
        for (n, coefficient) in enumerate(section):
            if n != 3:
                #print(f"{float2q31(coefficient):#010x}, ",end="")
                coefficient = coefficient/2
                if n > 3:
                    coefficient = -1.0 * coefficient
                c = float2q31(coefficient)
                print(f"{c:#010x}, ",end="")
                #print(f"{q312float(c)}, ",end="")
        print("")
    print("};")
    print(f"q31_t FilterState[{4*num_stages}];")

if __name__ == "__main__":
    #sos = ellip(fs, 4.5e3, 5e3, 1, 60)
    #sos = ellip(fs,1.35e3, 1.5e3, 1, 80)
    #sos = ellip_bp()
    #sos = ellip(fs,200,250,1,80)

    sos = cheby(fs,1.35e3, 1.5e3, 1, 80)
    sos2cmsis(sos)
   
    plt.figure()
    [w, h] = signal.sosfreqz(sos, fs=fs)
    plt.semilogx(w,20*np.log10(np.abs(h)))
    plt.grid()
    plt.show()
