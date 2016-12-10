#include "debug.h"

void debug_init() {
  PORTSetPinsDigitalOut(DEBUG_LED_PORT, DEBUG_LED_PIN);
  PORTClearBits(DEBUG_LED_PORT, DEBUG_LED_PIN);
}

void debug_led_on() {
  PORTSetBits(DEBUG_LED_PORT, DEBUG_LED_PIN);
}

void debug_led_off() {
  PORTClearBits(DEBUG_LED_PORT, DEBUG_LED_PIN);
}

void debug_led_toggle() {
  PORTClearBits(DEBUG_LED_PORT, DEBUG_LED_PIN);
}

void debug_error_handler() {
  PORTSetBits(DEBUG_LED_PORT, DEBUG_LED_PIN);
  while(1);
}

