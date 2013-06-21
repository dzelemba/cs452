#include "uart.h"
#include "ts7200.h"
#include "debug.h"

/*
 * The UARTs are initialized by RedBoot to the following state
 * 	115,200 bps
 * 	8 bits
 * 	no parity
 * 	fifos enabled
 */

int get_base(int channel) {
  switch( channel ) {
    case COM1:
      return (UART1_BASE);
    case COM2:
      return (UART2_BASE);
    default:
      ERROR("uart.c: get_base was given invalid channel: %d\n", channel);
      return 0;
  }
}

int* get_line(int channel, int offset) {
  return (int *)(get_base(channel) + offset);
}

void ua_setfifo(int channel, int state) {
  int* line = get_line(channel, UART_LCRH_OFFSET);

  int buf = *line;
  buf = state ? buf | FEN_MASK : buf & ~FEN_MASK;
  *line = buf;
}

void ua_setspeed(int channel, int speed) {
  int* high = get_line(channel, UART_LCRM_OFFSET);
  int* low = get_line(channel, UART_LCRL_OFFSET);
	switch( speed ) {
	  case 115200:
		  *high = 0x0;
		  *low = 0x3;
      break;
	  case 2400:
		  *high = 0x0;
		  *low = 0xbf;
      break;
	  default:
      ASSERT(0, "uart.c: ua_setspeed was given invalid speed");
	}
}

void ua_setstopbits(int channel, int state) {
  int *line = get_line(channel, UART_LCRH_OFFSET);

  int buf = *line;
  buf = state ? buf | STP2_MASK : buf & ~STP2_MASK;
  *line = buf;
}

void ua_enableinterrupts(int channel, int mask) {
  int* line = get_line(channel, UART_CTLR_OFFSET);

  int buf = *line;
  buf |= mask;
  *line = buf;
}

void ua_disableinterrupts(int channel, int mask) {
  int* line = get_line(channel, UART_CTLR_OFFSET);

  int buf = *line;
  buf &= ~mask;
  *line = buf;

}

int ua_get_intr_status(int channel) {
  return *get_line(channel, UART_INTR_OFFSET);
}

void ua_clearCTSintr(int channel) {
  *get_line(channel, UART_INTR_OFFSET) = 1;
}

char ua_getc(int channel) {
  ASSERT(*get_line(channel, UART_FLAG_OFFSET) & RXFF_MASK, "uart.c: ua_getc: no data ready");

  return *get_line(channel, UART_DATA_OFFSET);
}

int ua_ready_to_send(int channel) {
  int flags_val = *get_line(channel, UART_FLAG_OFFSET);

  switch(channel) {
  case COM1:
    return !(flags_val & TXFF_MASK) && (flags_val & CTS_MASK);
  case COM2:
    return !(flags_val & TXFF_MASK);
  default:
    return 0;
  }
}

void ua_putc(int channel, char c) {
  ASSERT(ua_ready_to_send(channel), "uart.c: ua_putc: not ready to send");

  *get_line(channel, UART_DATA_OFFSET) = c;
  ua_enableinterrupts(channel, TIEN_MASK);
}
