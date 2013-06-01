#include "timer.h"
#include "ts7200.h"

#define START_TICK 0xFFFFFFFF

void init_time() {
  unsigned int start_time = START_TICK;
  unsigned int* timer_load = (unsigned int*)(TIMER3_BASE + LDR_OFFSET);
  *timer_load = start_time;

  char* timer_crtl = (char*)(TIMER3_BASE + CRTL_OFFSET);
  *timer_crtl = ENABLE_MASK | CLKSEL_MASK;
}

unsigned int ticks() {
  unsigned int start_time = START_TICK;
  unsigned int* timer_value = (unsigned int*)(TIMER3_BASE + VAL_OFFSET);
  return start_time - *timer_value;
}

unsigned int ticks_to_micros(unsigned int ticks) {
  return (unsigned int)((float)ticks * (1000.0 / 508.0));
}
