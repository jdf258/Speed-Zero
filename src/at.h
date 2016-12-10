#ifndef __AT_H__
#define __AT_H__


#include <stdint.h>
#include <plib.h>

// defunct due to function macro and preprocessor concatenation (Macros. Suck.)
// NOTE Defunct due to PPS function macro.
//      I could get around it using another level of preprocessor macros, but
//      then what would I be? Some jerk who uses preprocessor macros for no good
//      reason.
#define AT_UART UART2
#define AT_UART_BAUDRATE 115200
#define AT_UART_TX  U2TX
#define AT_UART_TX_GRP 4
#define AT_UART_TX_PIN RPB10
#define AT_UART_RX U2RX
#define AT_UART_RX_GRP 2
#define AT_UART_RX_PIN RPB11

typedef enum {
  AT_OK = 0,
  AT_SEND_OK,
  AT_ARROW,
  AT_ERROR,
  AT_BUSY,
  AT_NO_CHANGE,
  AT_CONNECT,
  AT_CLOSED,
  AT_MESSAGE,
  AT_RESPONSE_NUM_ITEMS
} AT_Response;

typedef struct {
  AT_Response header;
  uint8_t *payload;
  uint16_t length;
  uint16_t capacity;
} AT_Packet;

void at_init();
void at_write_raw(uint8_t *data, uint16_t length);
void at_write_cmd(const char *cmd);
void at_read_packet(AT_Packet *packet);
void at_write_cmd_until(const char *cmd, AT_Response response, AT_Packet *packet);

#endif
