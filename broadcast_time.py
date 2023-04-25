#!/bin/python3
import serial
import sys
import time

port = sys.argv[1] if len(sys.argv) > 1 else '/dev/ttyUSB0'
with serial.Serial(port, 115200, timeout=3) as ser:
	ser.dtr = False # Drop DTR
	time.sleep(0.022)    # Read somewhere that 22ms is what the UI does.
	ser.dtr = True  # UP the DTR back

	# wait for boot garbage to be sent
	line = None
	while line != b'':
		line = ser.readline()
		print(line)

	time.sleep(2)

	# set current time, wait for it to be sent to all nodes
	now = int(time.time())
	ser.write(b"log_level BLE_MESH info\n")
	print(ser.readline())
	ser.write(b"log_level badge/mesh verbose\n")
	print(ser.readline())
	ser.write(b"mesh-admin set-time %u\n" % now)
	print(ser.readline())

	while True:
		line = ser.readline()
		if line != b'':
			print(line)
		# if b'nsec>' in line:
		# 	break
