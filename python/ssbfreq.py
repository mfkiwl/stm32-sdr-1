#!/usr/bin/env python3

import serial
import sys

offset = 0

#wm = 5000
#wl = 300
#wh = 3000
#B = wh-wl
#wxusb = int(wm + B/2 - wh)
#wxlsb = int(wm - B/2 + wh)

fxlsb = 6650;
fxusb = 3350;
fxcw = 4250;

with serial.Serial("/dev/ttyACM0", 115200) as ser:
    freq = int(float(sys.argv[2])*1e3)
    if sys.argv[1] == "lsb":
        freq = freq - fxlsb
    elif sys.argv[1] == "usb":
        freq = freq - fxusb
    elif sys.argv[1] == "cw":
        freq = freq - fxusb
    elif sys.argv[1] == "am":
        freq = freq #- 12000

    print(freq)

    ser.write(f"#{freq}\n".encode("utf-8"))
