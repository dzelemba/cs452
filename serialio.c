#include "serialio.h"
#include "uart.h"

void init_train_uart() {
  ua_setspeed(COM1, 2400);
  ua_setstopbits(COM1, ON);
  ua_setfifo(COM1, OFF);

  ua_setfifo(COM2, OFF);
}
