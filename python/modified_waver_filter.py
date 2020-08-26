#!/usr/bin/env

import numpy as np
from scipy import signal
import matplotlib.pyplot as plt
import struct

fs = 48e3

def ellip_bp(fs,fm,B,rpass,gstop):
    fpass = np.array([fm-B/2, fm+B/2])
    fstop = np.array([fm-B/2-0.1*B, fm+B/2+0.1*B])
    wpass = 2*fpass/fs
    wstop = 2*fstop/fs
    [order, wn] = signal.ellipord(wpass,wstop,rpass,gstop)
    sos = signal.ellip(order,rpass,gstop,wn,btype='bandpass',output='sos')
    return sos

def tuning_offset(fl,fh,fm):
    B2 = (fh-fl)/2
    fxusb = fm+B2-fh
    fxlsb = fm-B2+fh
    return (fxusb, fxlsb)

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
                coefficient = coefficient/2.0
                if n > 3:
                    coefficient = -1.0 * coefficient
                c = float2q31(coefficient)
                print(f"{c:#010x},",end="")
                #print(f"{q312float(c)}, ",end="")
        print("")
    print("};")
    print(f"q31_t FilterState[{4*num_stages}];")

def offset2c(fs,fxusb,fxlsb):
    print(f"const uint32_t dphi_usb = {float2q31(fxusb/fs):#010x};")
    print(f"const uint32_t dphi_lsb = {float2q31(fxlsb/fs):#010x};")


# USB/LSB
#sos = ellip_bp(fs,5e3,2.7e3,1,80)
#(fxusb, fxlsb) = tuning_offset(3e2,3e3,5e3)

# CW
sos = ellip_bp(fs,5e3,2e2,1,80)
(fxusb, fxlsb) = tuning_offset(650,850,5e3)

print(f"USB tuning offset: {fxusb}")
print(f"LSB tuning offset: {fxlsb}")

sos2cmsis(sos)
offset2c(fs,fxusb,fxlsb)



plt.figure()
[w, h] = signal.sosfreqz(sos, fs=fs)
plt.semilogx(w,20*np.log10(np.abs(h)))
plt.grid()
plt.show()
