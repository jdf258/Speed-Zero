#!/usr/local/bin/python
import time
import serial

# configure the serial connections (the parameters differs on the device you are connecting to)
# serial parameters
TIMESTAMP = str(int(time.time()))
LOG_FILE ="log/serial"+TIMESTAMP+'.log'

#"/dev/tty.usbserial-FT9AJZ83"
SERIAL_BUFFER = 100
BAUDRATE      = 9600
XONXOFF       = True # software flow control
RTSCTS        = False # hardware flow control
TIMEOUT       = 1
PARITY        = serial.PARITY_EVEN
BYTESIZE      = serial.SEVENBITS
STOPBITS      = serial.STOPBITS_TWO
num_chars     = 100


class CNC_Serial:
  def __init__(self, usb_port_name):
    self.serial   = serial.Serial(port=usb_port_name, baudrate=BAUDRATE, xonxoff=XONXOFF,
                                  rtscts=RTSCTS, timeout=TIMEOUT, parity=PARITY,
                                  bytesize=BYTESIZE, stopbits=STOPBITS)
    self.x = 0
    self.y = 0
    self.z = 0

  def wait_for_connection(self):
    with open(LOG_FILE, 'w+') as log_file:
        flag = True
        print("Hello")
        while flag:
            buff = self.serial.read(num_chars)
            if(len(buff) > 0):
                print(buff)
                log_file.write("Init Handshake, Received filename"+buff)
                flag = False
        return
  def send_start_sequence(serial_conn):
    for s in ["I$zero.min %", "O9999", "G17 G40 G80 G90 G20","F50", "G56H14","M1"]:
        print("Sending: " + s)
        serial_conn.write(s+"\r\n")

  def send_end_sequence(self):
    print("Sending the M30")
    self.serial.write("M30"+"\r\n")
    self.serial.write("%"+"\r\n")

  def move(self, x = None,y = None,z = None):

    print("G0X%.4fY%.4fZ%.4f" % (self.x, self.y, self.z))
    if(x != None):
      self.x += x
      print("G0X%.4f\r\n" % self.x)
      self.serial.write("G0X%.4f\r\n" % self.x)
    if(y != None):
      self.y += y
      print("G0Y%.4f\r\n" % self.y)
      self.serial.write("G0Y%.4f\r\n" % self.y)
    if(z != None):
      self.z += z
      print("G0Z%.4f\r\n" % self.z)
      self.serial.write("G0Z%.4f\r\n" % self.z)
    self.serial.write("M1\r\n")
    #print("G0X%.4fY%.4fZ%.4f" % (self.x, self.y, self.z))
    #self.serial.write("G0X%.4fY%.4f\r\n" % (self.x, self.y, self.z))

  def write(self, msg):
    self.serial.write(msg)
  def close(self):
    self.serial.close()