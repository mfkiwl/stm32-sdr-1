#!/usr/bin/env python3

import serial
import sys

cmd_name = sys.argv[1]
cmd_type = sys.argv[2]

if cmd_name == "freq":
    cmd_value = int(float(sys.argv[3])*1e3)
else:
    cmd_value = int(sys.argv[3])

with serial.Serial("/dev/ttyACM0", 115200) as ser:
    cmd = f"#{cmd_name} {cmd_type} {cmd_value:d}\n"
    print(cmd.encode('utf-8'))
    ser.write(cmd.encode("utf-8"))
    #print(ser.readline().decode("utf-8"))
