#ifndef __INDUCTION_H__
#define __INDUCTION_H__

#include "plib.h"

#define INDUCTION_PORT IOPORT_B
#define INDUCTION_PIN BIT_13

void induction_init();
void induction_wifi_send();

#endif
