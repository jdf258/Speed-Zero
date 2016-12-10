#ifndef __DEBUG_H__
#define __DEBUG_H__


#include <plib.h>


#define DEBUG_LED_PORT IOPORT_A
#define DEBUG_LED_PIN BIT_0


void debug_init();

void debug_led_on();

void debug_led_off();

void debug_led_toggle();

// turn on the debug led and loop forever
void debug_error_handler();

#endif
