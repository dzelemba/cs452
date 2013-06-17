#ifndef __UART_H__
#define __UART_H__

#define COM1  0
#define COM2  1

#define ON  1
#define OFF 0

void ua_setfifo(int channel, int state);

void ua_setspeed(int channel, int speed);

void ua_setstopbits(int channel, int state);

void ua_enableinterrupts(int channel, int mask);

void ua_disableinterrupts(int channel, int mask);

int ua_get_intr_status(int channel);

void ua_clearCTSintr(int channel);

int ua_ready_to_send(int channel);

char ua_getc(int channel);

void ua_putc(int channel, char c);

#endif
