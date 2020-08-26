#!/usr/bin/env python3

import numpy as np
from scipy import signal
from scipy.io import wavfile

USB = False

def ellip(fs, fpass, fstop, gpass, gstop):
    wpass = 2*fpass/fs
    wstop = 2*fstop/fs
    [order, wn] = signal.ellipord(wpass, wstop, gpass, gstop)
    sos = signal.ellip(order, gpass, gstop, [wn], output='sos')
    return sos


def ssb_mod(samples, fs, fc, filter_sos, usb=True):
    sc_I = np.cos(2*np.pi*1.65e3/fs*np.arange(len(samples)))
    sc_Q = np.sin(2*np.pi*1.65e3/fs*np.arange(len(samples)))

    x_I = samples * sc_I
    x_Q = samples * sc_Q

    y_I = signal.sosfilt(filter_sos, x_I)
    y_Q = signal.sosfilt(filter_sos, x_Q)

    flo = 0
    if usb:
        flo = fc+1.65e3
    else:
        flo = fc-1.65e3

    lo_I = np.cos(2*np.pi*flo/fs*np.arange(len(y_I)))
    lo_Q = np.sin(2*np.pi*flo/fs*np.arange(len(y_Q)))
    z_I = lo_I * y_I
    z_Q = lo_Q * y_Q

    if usb:
        return z_I + z_Q
    else:
        return z_I - z_Q

def ssb_demod(I, Q,fs, fc, filter_sos, usb=True):
    flo = 0
    if usb:
        flo = fc+1.65e3
    else:
        flo = fc-1.65e3

    lo_I = np.cos(2*np.pi*flo/fs*np.arange(len(I)))
    lo_Q = np.sin(2*np.pi*flo/fs*np.arange(len(Q)))
    x_I = I * lo_I
    x_Q = Q * lo_Q

    y_I = signal.sosfilt(filter_sos, x_I)
    y_Q = signal.sosfilt(filter_sos, x_Q)

    sc_I = np.cos(2*np.pi*1.65e3/fs*np.arange(len(y_I)))
    sc_Q = np.sin(2*np.pi*1.65e3/fs*np.arange(len(y_Q)))

    z_I = y_I * sc_I
    z_Q = y_Q * sc_Q

    if usb:
        return z_I + z_Q
    else:
        return z_I - z_Q


(fs, baseband_input) = wavfile.read('speech.wav')

baseband_input = baseband_input / 32768.0

# compute low pass filter
sos = ellip(fs, 1.35e3, 1.5e3, 1, 80)

ssb_signal = ssb_mod(baseband_input, fs, 8.5e3, sos, usb=USB)

lo_I = np.cos(2*np.pi*2e3/fs*np.arange(len(ssb_signal)))
lo_Q = -np.sin(2*np.pi*2e3/fs*np.arange(len(ssb_signal)))
I = ssb_signal * lo_I
Q = ssb_signal * lo_Q

#ssb_signal = ssb_signal * losignal
out = np.reshape(np.vstack((I.astype('float32'),Q.astype('float32'))), (-1,2))
wavfile.write('out.wav', fs, out)
wavfile.write('ssb_signal.wav', fs, ssb_signal)
#a = np.reshape(np.vstack((I,Q)),(-1,2))
#print(a.shape)

baseband_output = ssb_demod(I, Q, fs, 3.5e3, sos, usb=USB)
wavfile.write('baseband_output.wav', fs, baseband_output)
