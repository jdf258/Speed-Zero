from wifi_protocol2 import *

conn = Wifi_Connection()
conn.open()
conn.resize(4)
conn.arducam('demo_3.jpg')
conn.close()