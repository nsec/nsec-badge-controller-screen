#!/bin/python3

import websocket
import json
import serial
import sys
from time import sleep
import threading

websocket.enableTrace(True)

ser = None

def serial_read_until_done():
    global ser
    line = None
    while line != b'':
        line = ser.readline()
        print(line)

def serial_func():
    global ser
    while True:
        try:
            port = sys.argv[1] if len(sys.argv) > 1 else '/dev/ttyUSB0'
            ser = serial.Serial(port, 115200, timeout=1)
            ser.dtr = False # Drop DTR
            sleep(0.022)    # Read somewhere that 22ms is what the UI does.
            ser.dtr = True  # UP the DTR back

            while True:
                line = ser.readline()
                if line != b'':
                    print(line.decode('utf-8'), end='')
        except Exception as e:
            print('serial thread exception', repr(e))
        sleep(5)

def console_func():
    global ser
    while True:
        try:
            while True:
                cmd = input()
                ser.write(bytes("%s\n" % cmd, encoding='utf-8'))
        except Exception as e:
            print('console thread exception', repr(e))
        sleep(5)

serial_thread = threading.Thread(target=serial_func)
serial_thread.start()
console_thread = threading.Thread(target=console_func)
console_thread.start()

def on_message(wsapp, message):
    global ser
    print(repr(message))

    # {
    #   "metadata":{
    #       "teamid":0,
    #       "team":{
    #           "name":"NorthSec",
    #           "country":"CA",
    #           "website":"https://nsec.io"
    #       },
    #       "score":{
    #           "submit_time":"2023-05-18T20:24:39.227514138Z",
    #           "value":10,
    #           "total":20
    #       },
    #       "type":"score-updated"
    #   },
    #   "server":"askgod03",
    #   "timestamp":"2023-05-18T20:24:39.227515718Z",
    #   "type":"timeline"
    # }

    data = json.loads(message)
    type = data["metadata"]["type"]
    if type == "score-updated":
        teamid = data["metadata"]["teamid"]
        points = data["metadata"]["score"]["value"]
        total = data["metadata"]["score"]["total"]

        msg = b"Congrats! You scored %u (you have %u total)" % (points, total)

        node_id = 0x7f00 + teamid
        command = b"mesh-admin ui-message 0x%04x \"%s\"\n" % (node_id, msg)
        ser.write(command)

    # {
    #   "metadata":{
    #       "score":null,
    #       "team":{
    #           "country":"CA",
    #           "name":"NorthSec Admins",
    #           "website":"https://nsec.io"
    #       },
    #       "teamid":0,
    #       "type":"team-updated"
    #   },
    #   "server":"askgod01",
    #   "timestamp":"2023-05-19T16:00:05.528857062Z",
    #   "type":"timeline"
    # }'

    if type == "team-updated":
        teamid = data["metadata"]["teamid"]
        team_name = data["metadata"]["team"]["name"]

        node_id = 0x7f00 + teamid
        command = b"mesh-admin set-name 0x%04x \"%s\"\n" % (node_id, bytes(team_name, encoding='utf-8'))
        ser.write(command)

wsapp = websocket.WebSocketApp("wss://askgod.nsec/1.0/events?type=timeline", on_message=on_message)
wsapp.run_forever()
print('*************** ws exited ***************')
