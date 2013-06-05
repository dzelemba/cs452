#include "timer.h"
#include "ts7200.h"

// 10ms on the 508kHz clock.
#define RESET_VALUE 50800

void init_timer() {
  unsigned int* timer_load = (unsigned int*)(TIMER3_BASE + LDR_OFFSET);
  *timer_load = RESET_VALUE;

  // Enable timer, set it to 508kHz, and put it in periodic mode.
  char* timer_crtl = (char*)(TIMER3_BASE + CRTL_OFFSET);
  *timer_crtl = ENABLE_MASK | CLKSEL_MASK | MODE_MASK;
}

void clear_timer_interrupt() {
  int* timer_clear = (int*)(TIMER3_BASE + CLR_OFFSET);
  *timer_clear = 1;
}

/*
 * 40-bit Debug Timer
 */

#define DEBUG_TIMER_HIGH 0x80810064
#define DEBUG_TIMER_LOW 0x80810060

void init_debug_timer() {
  unsigned int* timer_crtl = (unsigned int*)(DEBUG_TIMER_HIGH);
  *timer_crtl = 0xffffffff;
}

unsigned int ticks() {
  unsigned int* timer_value = (unsigned int*)(DEBUG_TIMER_LOW);
  return *timer_value;
}

unsigned int ticks_to_micros(unsigned int ticks) {
  return ((ticks * 1000) / 983);
  //return (unsigned int)((float)ticks * (1000.0 / 508.0));
}
