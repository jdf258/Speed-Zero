from threading import Thread
from threading import Lock
from wifi_interface import *
import socket
import logging
import sys
import binascii
import time

default_host = '192.168.4.1'
default_port = 80

codec = 'utf-8'

logger = logging.getLogger('Wifi')
logger_handler = logging.StreamHandler(sys.stdout)
logger_formatter = logging.Formatter('%(name)s:%(levelname)s:%(message)s')
logger_handler.setFormatter(logger_formatter)
logger.addHandler(logger_handler)
logger.setLevel(logging.DEBUG)


class Wifi_Packet:

    def __init__(self):
        self.header = bytes([0])
        self.payload = b''

    def serialize(self):
        return self.header + bytes([len(self.payload)]) + self.payload


class Wifi_Connection:
    def __init__(self, address = (default_host, default_port)):
        self.address = address
        self.conn = None

    def open(self):
        if not self.conn:
            self.conn = socket.create_connection(self.address)
        time.sleep(4)

    def close(self):
        if self.conn:
            self.conn.close()
            self.conn = None

    def send(self, packet):
        if not self.conn:
            raise RuntimeError('Connection not open')
        logger.debug('send: ' +  binascii.hexlify(packet.serialize()).decode(codec))
        self.conn.send(packet.serialize())

    def recv(self, debug_filter = True):
        while True:
            packet = Wifi_Packet()
            packet.header = self.conn.recv(1)
            length = int.from_bytes(self.conn.recv(1), byteorder='little')
            packet.payload = self.conn.recv(length)

            if (packet.header == WIFI_DEBUG):
                logger.debug('Device: ' + packet.payload.decode(codec))
                if not debug_filter:
                    return packet
            else:
                logger.debug('recv: ' + binascii.hexlify(packet.serialize()).decode(codec))
                return packet
    #size of the image. 0->8
    def resize(self, size):
        if size > 8:
            logger.error('resize: too big')
            return
        send_packet = Wifi_Packet()
        send_packet.header = WIFI_RESIZE_REQ
        send_packet.payload = bytes([size])
        self.send(send_packet)
        while True:
            recv_packet = self.recv()
            if recv_packet.header == WIFI_RESIZE_ACK:
                logger.info('resize: done')
                break

    def arducam(self, filename):
        fout = open(filename, 'wb')
        send_packet = Wifi_Packet()
        send_packet.header = WIFI_ARDUCAM_REQ
        self.send(send_packet)
        while True:
            recv_packet = self.recv()
            if(recv_packet.header == WIFI_ARDUCAM_DATA):
                fout.write(recv_packet.payload)
            elif(recv_packet.header == WIFI_ARDUCAM_END):
                fout.close()
                logger.info('arducam: done')
                break

    def induction(self):
        send_packet = Wifi_Packet()
        send_packet.header = WIFI_INDUCTION_REQ
        self.send(send_packet)
        while True:
            recv_packet = self.recv()
            if(recv_packet.header == WIFI_INDUCTION_DATA):
                reading = int.from_bytes(recv_packet.payload, byteorder='little')
                logger.info('induction: %d' %reading)
                return reading

    def reset(self):
        send_packet = Wifi_Packet()
        send_packet.header = WIFI_RESET_REQ
        self.send(send_packet)
        while True:
            recv_packet = self.recv()
            if(recv_packet.header == WIFI_RESET_ACK):
                conn.close()
                logger.info('reset: done')
                break

