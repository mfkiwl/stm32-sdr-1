#!/usr/bin/env python3

import numpy as np
from scipy import signal
import matplotlib.pyplot as plt

def cos_dho(f,fs,invert=False):
    W0 = 2*np.pi*f/fs
    if invert:
        return [-1,+np.cos(W0),0,1,-2*np.cos(W0),1]
    else:
        return [1,-np.cos(W0),0,1,-2*np.cos(W0),1]

def sin_dho(f,fs,invert=False):
    W0 = 2*np.pi*f/fs
    if invert:
        return [0,-np.sin(W0),0,1,-2*np.cos(W0),1]
    else:
        return [0,np.sin(W0),0,1,-2*np.cos(W0),1]

def complex_dho(f,fs,invert=False):
    W0 = 2*np.pi*f/fs
    if invert:
        return [1,0,0,1,-np.exp(-1j*W0),0]
    else:
        return [1,0,0,1,-np.exp(1j*W0),0]

#sos = cos_dho(2.4e3, 48e3,invert=True)
sos = complex_dho(2.4e3, 48e3,invert=True)

x = np.zeros(100)
x[0] = 1
y = signal.sosfilt(sos,x)

plt.stem(np.real(y),linefmt='blue',markerfmt='blue')
plt.stem(np.imag(y),linefmt='red',markerfmt='red')
plt.grid()
plt.show()
