#!/usr/bin/env python3

import serial
import sys

with serial.Serial("/dev/ttyACM0", 115200) as ser:
    freq = int(float(sys.argv[1])*1e3)
    ser.write(f"#{freq}\n".encode("utf-8"))
