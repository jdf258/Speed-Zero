#!/usr/local/bin/python

from wifi_interface import *
from wifi_protocol import Wifi_Packet
from wifi_protocol import Wifi_Connection
from wifi_protocol import codec
from cnc_serial import CNC_Serial
from hole_detection import *
import logging
import sys
import time
import math


CAMERA_OFFSET_X = 0
CAMERA_OFFSET_Y = 0
CAMERA_ORIGIN_X = 160/2
CAMERA_ORIGIN_Y = 120/2

def image_test(conn):
    fout = open('image.jpg', 'wb')
    send_packet = Wifi_Packet()
    send_packet.header = WIFI_ARDUCAM_REQ
    conn.send(send_packet)
    while True:
        recv_packet = conn.recv()
        if (recv_packet.header == WIFI_ARDUCAM_DATA):
            fout.write(recv_packet.payload)
        if (recv_packet.header == WIFI_ARDUCAM_END):
            fout.close()
            break
    conn.close()

#returns something in units of pixels/inch
def pixel_to_distance(x1,y1,x2,y2,distance):
  pixel_dist = math.sqrt((x2-x1)**2 + (y2-y1)**2)
  return pixel_dist/distance

#finds the inverse matrix transform
def matrix_inverse(matrix):
  a = matrix[0][0]
  b = matrix[0][1]
  c = matrix[1][0]
  d = matrix[1][1]
  det = a*d - b*c
  return [[d/det, -1*b/det], [-1*c/det, a/det]]

def rotate_points(matrix, points):
  x = points[0]*matrix[0][0]+points[1]*matrix[0][1]
  y = points[0]*matrix[1][0]+points[1]*matrix[1][1]
  return (x,y)

#Convert the camera coordinates from pixels to cnc distances
def convert_coords(camera_coords, pixel_dist, rotation_matrix):
  x,y = camera_coords

  x,y = rotate_points(rotation_matrix, [x,y])

  x,y = (x/pixel_dist, y/pixel_dist)

  return x,y

def find_theta(x,y,x_o,y_o):
  return math.asin((y_o-y)/x_o)


X0 = 0
X1 = 0
Y0 = 0
Y1 = 2
Z0 = 0
Z1 = 0

if __name__ == '__main__':
  conn = Wifi_Connection()
  conn.open()
  time.sleep(10)
  ser = CNC_Serial("/dev/tty.usbserial-FT9AJZ83")

  #Init phase
  ser.wait_for_connection()
  print("Received a CNC connection.")
  ser.send_start_sequence()
  print("Sending the start sequence")

  #Camera "Zero-ing" Phase
  ser.move(X0, Y0, Z0)
  image_test(conn)
  r, x_c1, y_c1 = process_image('image.jpg')
  print("Radius: %s X: %s Y: %s", r, x_c, y_c)
  #have the initial radius and stuff
  #move 2 inches away in the Y direction
  ser.move(X1, Y1, Z1)
  image_test(conn)

  r, x_c2, y_c2 = process_image('image.jpg')
  #Have sample images now to find out the rotation offset of the camera
  #TODO: Verify that this line is correct
  pixel_dist = pixel_to_distance(x_c1, y_c1, x_c2, y_c2, (Y2-Y1))

  #set the origin of the camera in the center of the circle
  camera_x, camera_y = (x_c2 - x_c1, y_c2 - y_c1)
  theta = find_theta(x_c2, y_c2, X1, Y1)
  print(theta)
  rotation_matrix = [ [math.cos(theta), -1*math.sin(theta)],
                      [math.sin(theta), math.cos(theta)]]

  #figure out how much you need to move in the X/Y space to get the camera centered on the image
  x_move, y_move = convert_coords(
                      (x_c1-CAMERA_ORIGIN_X, y_c1 - CAMERA_ORIGIN_Y),
                      pixel_dist, rotation_matrix)

  ser.move(x_move, y_move)
