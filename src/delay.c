#include "main.h"
#include "delay.h"
#include <plib.h>

volatile unsigned int time_tick_millsec = 0;

void delay_init() {
  OpenTimer5(T5_ON  | T5_SOURCE_INT | T5_PS_1_1 , PB_CLOCK/1000);
  ConfigIntTimer5(T5_INT_ON | T5_INT_PRIOR_2);
  mT5ClearIntFlag();
}

// TODO add wrap-around functionality
void delay(unsigned int msec) {
  // NOTE unsafe. should disable the interrupt before assigning stop_time.
  volatile unsigned int stop_time = time_tick_millsec + msec;

  while(time_tick_millsec < stop_time);
}

void __ISR(_TIMER_5_VECTOR, IPL2AUTO) Timer5Handler(void) {
  mT5ClearIntFlag();
  time_tick_millsec++;
}
