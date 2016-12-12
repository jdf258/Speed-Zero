#!/usr/local/bin/python


from cnc_serial import CNC_Serial
import logging
import sys
import time
import math

X0 = 0
X1 = 2
Y0 = 0
Y1 = 0
Z0 = 0
Z1 = 0

if __name__ == '__main__':
  ser = CNC_Serial("/dev/tty.usbserial-FT9AJZ83")
  #Init phase
  ser.wait_for_connection()
  print("Received a CNC connection.")
  ser.send_start_sequence()
  print("Sending the start sequence")
  #Camera "Zero-ing" Phase
  ser.move(X0, Y0)
  #have the initial radius and stuff
  #move 2 inches away in the X direction
  ser.move(X1, Y1)
  print("sending end sequence")
  ser.send_end_sequence()
