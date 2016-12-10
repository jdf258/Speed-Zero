#ifndef __DELAY_H__
#define __DELAY_H__

extern volatile unsigned int time_tick_millsec;

void delay_init();

void delay(unsigned int msec);


#endif
