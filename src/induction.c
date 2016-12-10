#include "induction.h"
#include "wifi_interface.h"
#include "wifi.h"

uint8_t packet_data[1] = {0};
Wifi_Packet packet;

void induction_init() {
  packet.header = WIFI_INDUCTION_DATA;
  packet.length = 1;
  packet.capacity = 1;
  packet.payload = packet_data;

  PORTSetPinsDigitalIn(INDUCTION_PORT, INDUCTION_PIN);
}

void induction_wifi_send() {
  packet.payload[0] = PORTReadBits(INDUCTION_PORT, INDUCTION_PIN);
  wifi_send(&packet);
}
