#!/usr/bin/env python3

import serial
import sys

with serial.Serial("/dev/ttyACM0", 115200) as ser:
    freq = int(float(sys.argv[2])*1e3)
    if sys.argv[1] == "lsb":
        freq = freq - 1650
    elif sys.argv[1] == "usb":
        freq = freq + 1650
    ser.write(f"#{freq}\n".encode("utf-8"))
