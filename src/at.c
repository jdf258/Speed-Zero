#include "main.h"
#include "at.h"
#include "debug.h"
#include "wifi.h"


static const char *at_response[AT_RESPONSE_NUM_ITEMS] = {
  "OK",
  "SEND OK",
  ">",
  "ERROR",
  "busy...",
  "no change",
  "0,CONNECT",
  "0,CLOSED",
  "+IPD",
};


const uint16_t at_response_last_index[] = {1, 6, 0, 4, 6, 8, 8, 7, 3};


void at_init() {
  // setup serial
  PPSOutput(4, RPB10, U2TX);
  PPSInput(2, U2RX, RPB11);
  UARTConfigure(AT_UART, UART_ENABLE_PINS_TX_RX_ONLY);
  UARTSetLineControl(AT_UART, UART_DATA_SIZE_8_BITS | UART_PARITY_NONE | UART_STOP_BITS_1);
  UARTSetDataRate(AT_UART, PB_CLOCK, AT_UART_BAUDRATE);
  UARTEnable(AT_UART, UART_ENABLE_FLAGS(UART_PERIPHERAL | UART_RX | UART_TX));
}


void at_write_raw(uint8_t *data, uint16_t length) {
  uint16_t index = 0;

  while(index < length) {
    while(!UARTTransmitterIsReady(AT_UART));
    UARTSendDataByte(AT_UART, data[index++]);
  }
}


void at_write_cmd(const char *cmd) {
  uint16_t index = 0;

  while(cmd[index] != 0) {
    while(!UARTTransmitterIsReady(AT_UART));
    UARTSendDataByte(AT_UART, cmd[index++]);
  }

  at_write_raw((uint8_t *)"\r\n", 2);
}

void at_read_packet(AT_Packet *packet) {
  uint8_t buffer[16] = {0};
  uint16_t buffer_index = 0;
  uint16_t possible = (1 << AT_RESPONSE_NUM_ITEMS) - 1;
  uint16_t possible_temp = 0;
  uint16_t possible_index = 0;
  uint8_t z = 0;

  while(!z) {
    while(!UARTReceivedDataIsAvailable(AT_UART));
    buffer[buffer_index] = UARTGetDataByte(AT_UART);

    if((buffer[buffer_index] == '\r') || (buffer[buffer_index] == '\n')) {
      possible = (1 << AT_RESPONSE_NUM_ITEMS) - 1;
      buffer_index = 0;
      continue;
    }

    possible_temp = possible;
    while(possible_temp) {
      possible_index = _ctz(possible_temp & (-possible_temp));

      if(buffer[buffer_index] == at_response[possible_index][buffer_index]) {
        if(buffer_index == at_response_last_index[possible_index]) {
          packet->header = possible_index;
          z = 1;
          break;
        }
      } else
        possible ^= 1 << possible_index;

      possible_temp ^= 1 << possible_index;
    }

    buffer_index++;
  }

  if(packet->header == AT_MESSAGE) {
    for(z = 0; z < 3; z++) {
      while(!UARTReceivedDataIsAvailable(AT_UART));
      UARTGetDataByte(AT_UART);
    }

    // read until ':' to get payload length
    buffer_index = 0;
    do {
      while(!UARTReceivedDataIsAvailable(AT_UART));
      buffer[buffer_index] = UARTGetDataByte(AT_UART);
    } while (buffer[buffer_index++] != ':');
    buffer[buffer_index-1] = 0;

    // parse payload length
    packet->length = atoi(buffer);

    // collect payload
    buffer_index = 0;
    while(buffer_index < packet->length) {
      while(!UARTReceivedDataIsAvailable(AT_UART));
      packet->payload[buffer_index++] = UARTGetDataByte(AT_UART);
    }
  }
}

void at_write_cmd_until(const char *cmd, AT_Response response, AT_Packet *packet) {
  do {
    at_write_cmd(cmd);
    at_read_packet(packet);
  } while (packet->header != response);
}
