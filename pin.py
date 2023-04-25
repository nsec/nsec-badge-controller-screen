#!/bin/python3
import serial
import sys
from time import sleep

port = sys.argv[1] if len(sys.argv) > 1 else '/dev/ttyUSB0'
with serial.Serial(port, 115200, timeout=3) as ser:
	ser.dtr = False # Drop DTR
	sleep(0.022)    # Read somewhere that 22ms is what the UI does.
	ser.dtr = True  # UP the DTR back

	line = None
	while line != b'':
		line = ser.readline()
		print(line)

	pin = 0
	while True:
		ser.write(b"debug enable %u\n" % pin)
		line = [ser.readline(), ser.readline(), ser.readline()]
		print(repr(line))
		if b'incorrect pin' not in line[1]:
			print('Found pin %u' % pin)
			break
		pin += 1
