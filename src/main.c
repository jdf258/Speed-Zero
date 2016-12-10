#include "main.h"
#include "debug.h"
#include "wifi_interface.h"
#include "wifi.h"
#include "induction.h"
#include "delay.h"
#include <plib.h>

// Clock config
// Source: brl4, pt_cornell_1_1.h
#pragma config FNOSC = FRCPLL, POSCMOD = OFF
#pragma config FPLLIDIV = DIV_2, FPLLMUL = MUL_20 //40 MHz
#pragma config FPBDIV = DIV_1, FPLLODIV = DIV_2 // PB 40 MHz
#pragma config FWDTEN = OFF,  FSOSCEN = OFF, JTAGEN = OFF

#define WIFI_PACKET_BUFFER_LENGTH 200
uint8_t wifi_packet_buffer[200] = {0};
Wifi_Packet wifi_packet;


void platform_test() {
    debug_led_toggle();
    delay(500);
}

void platform_test2() {
    wifi_send_debug("Rikkiiii");
    delay(500);
}

void wifi_debug_test() {
    wifi_send_debug("Hello");
    delay(1000);
}

void wifi_challenge0() {
    do {
    	wifi_recv(&wifi_packet);
    } while(wifi_packet.header != WIFI_TEST);

    wifi_send_debug("Thanks guy!");
}

void wifi_challenge1() {
  uint8_t temp = 0;
    do {
      wifi_recv(&wifi_packet);
    } while(wifi_packet.header != WIFI_TEST);

    temp = wifi_packet.payload[0];
    wifi_packet.payload[0] = wifi_packet.payload[1];
    wifi_packet.payload[1] = temp;
    wifi_packet.length = 2;

    wifi_send(&wifi_packet);
}

// TODO idk how to derive an image on the python side
void arducam_image_test() {
  do {
    wifi_recv(&wifi_packet);
  } while(wifi_packet.header != WIFI_ARDUCAM_REQ);

  arducam_wifi_send();
}

void slave_mode() {
  while(1) {
    debug_led_on();
    wifi_recv(&wifi_packet);
    debug_led_off();

    switch(wifi_packet.header) {
    case WIFI_RESIZE_REQ:
      ov2640_set_jpeg_size(wifi_packet.payload[0]);
      wifi_packet.header = WIFI_RESIZE_ACK;
      wifi_packet.length = 0;
      wifi_send(&wifi_packet);
      break;
    case WIFI_ARDUCAM_REQ:
      arducam_start_capture();
      while(!arducam_capture_done());
      arducam_wifi_send();
      break;
    case WIFI_INDUCTION_REQ:
      induction_wifi_send();
      break;f
    case WIFI_RESET_REQ:
      wifi_packet.header = WIFI_RESET_ACK;
      wifi_packet.length =0;
      wifi_send(&wifi_packet);
      delay(2000); // wait for wifi connection close
      SoftReset();
      break;
    }
  }
}


void init(void) {
  // configure clocks
  SYSTEMConfig(SYS_CLOCK, SYS_CFG_WAIT_STATES | SYS_CFG_PCACHE);

  // setup outputs as digital. analog by default.
  ANSELA = 0;
  ANSELB = 0;

  // enable interrupts
  INTEnableSystemMultiVectoredInt();

  // initialize submodules
  delay_init();
  debug_init();
  delay(2000); // wait for power to stabilize
  wifi_init();
  arducam_init();
  induction_init();

  // setup wifi resource
  wifi_packet.payload = wifi_packet_buffer;
  wifi_packet.capacity = WIFI_PACKET_BUFFER_LENGTH;
}

void main() {
  init();

  slave_mode();
}
