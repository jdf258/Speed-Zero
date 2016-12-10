#ifndef __WIFI_H__
#define __WIFI_H__

#include <stdint.h>

#define WIFI_DEBUG_ENABLE

typedef struct {
  uint8_t header;
  uint8_t length;
  uint8_t capacity;
  uint8_t *payload;
} Wifi_Packet;

#define WIFI_DEBUG_BUFFER_LENGTH 200
extern char wifi_debug_buffer[WIFI_DEBUG_BUFFER_LENGTH];

void wifi_init();


void wifi_send(Wifi_Packet *packet);


void wifi_recv(Wifi_Packet *packet);


void wifi_send_debug(const char *msg);


#endif
