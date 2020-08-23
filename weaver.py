#!/usr/bin/env python3

import numpy as np
from scipy import signal
from scipy.io import wavfile

def ellip(fs, fpass, fstop, gpass, gstop):
    wpass = 2*fpass/fs
    wstop = 2*fstop/fs
    [order, wn] = signal.ellipord(wpass, wstop, gpass, gstop)
    sos = signal.ellip(order, gpass, gstop, [wn], output='sos')
    return sos


def ssb_mod(samples, fs, fc, filter_sos, usb=True):
    audio_sc = np.exp(-2j*np.pi*1.65e3/fs*np.arange(len(samples)))

    if not usb:
        audio_sc = np.conj(audio_sc)

    x = samples * audio_sc
    y = signal.sosfilt(filter_sos, x)

    flo = 0
    if usb:
        flo = fc+1.65e3
    else:
        flo = fc-1.65e3

    lo_signal = np.exp(2j*np.pi*flo/fs*np.arange(len(y)))
    z = lo_signal * y

    return np.real(z) + np.imag(z)

def ssb_demod(samples, fs, fc, filter_sos, usb=True):
    flo = 0
    if usb:
        flo = fc+1.65e3
    else:
        flo = fc-1.65e3

    lo_signal = np.exp(-2j*np.pi*flo/fs*np.arange(len(samples)))
    x = samples * lo_signal
    y = signal.sosfilt(filter_sos, x)

    audio_sc = np.exp(2j*np.pi*1.65e3/fs*np.arange(len(y)))

    if not usb:
        audio_sc = np.conj(audio_sc)

    z = y * audio_sc

    return np.real(z) + np.imag(z)


(fs, baseband_input) = wavfile.read('speech.wav')

baseband_input = baseband_input / 32768.0

# compute low pass filter
sos = ellip(fs, 1.35e3, 1.5e3, 1, 80)

ssb_signal = ssb_mod(baseband_input, fs, 5e3, sos, usb=False)
wavfile.write('ssb_signal.wav', fs, ssb_signal)

baseband_output = ssb_demod(ssb_signal, fs, 5e3, sos, usb=False)
wavfile.write('baseband_output.wav', fs, baseband_output)
