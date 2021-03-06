#ifndef __TIMER_H__
#define __TIMER_H__

/* 32-Bit Timer */

void init_timer();

void clear_timer_interrupt();

/* Uses the 40-bit debug timer */

void init_debug_timer();

unsigned int edges();

unsigned int edges_to_micros(unsigned int edges);

#endif
