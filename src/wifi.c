#include "stdlib.h"
#include "string.h"
#include "at.h"
#include "wifi_interface.h"
#include "wifi.h"
#include "debug.h"


char wifi_debug_buffer[WIFI_DEBUG_BUFFER_LENGTH];


#define AT_PACKET_PAYLOAD_LENGTH 255
uint8_t at_packet_payload[AT_PACKET_PAYLOAD_LENGTH] = {0};
AT_Packet at_packet;

void wifi_init() {
  at_init();

  at_packet.payload = at_packet_payload;
  at_packet.capacity = AT_PACKET_PAYLOAD_LENGTH;

  // setup wifi and server
  at_write_cmd_until("ATE1", AT_OK, &at_packet);
  at_write_cmd_until("AT+CWMODE=2", AT_OK, &at_packet);
  at_write_cmd_until("AT+CIPMUX=1", AT_OK, &at_packet);
  at_write_cmd_until("AT+CIPSERVER=1,80", AT_OK, &at_packet);

  debug_led_on();
  do {
    at_read_packet(&at_packet);
  } while (at_packet.header != AT_CONNECT);
  debug_led_off();
}

void wifi_send(Wifi_Packet *packet) {
  uint8_t buffer[16] = {0};
  uint8_t index = 0;

  sprintf(buffer, "AT+CIPSEND=0,%d", packet->length + 2);
  at_write_cmd(buffer);

  do {
    at_read_packet(&at_packet);
  } while(at_packet.header != AT_ARROW);

  // send serialized data
  at_write_raw(&packet->header, 1);
  at_write_raw(&packet->length, 1);
  at_write_raw(packet->payload, packet->length);

  do {
    at_read_packet(&at_packet);
  } while(at_packet.header != AT_SEND_OK);
}


void wifi_recv(Wifi_Packet *packet) {
  int index = 0;

  do {
    at_read_packet(&at_packet);
  } while(at_packet.header != AT_MESSAGE);

  packet->header = at_packet.payload[0];
  packet->length = at_packet.payload[1];

  for(;index < packet->length; index++)
    packet->payload[index] = at_packet.payload[index+2];

  return;
}

void wifi_send_debug(const char *msg) {
  #ifdef WIFI_DEBUG_ENABLE
  Wifi_Packet packet;

  packet.header = WIFI_DEBUG;
  packet.length = strlen(msg);
  packet.payload = (uint8_t *)msg;

  wifi_send(&packet);
  #endif
}

