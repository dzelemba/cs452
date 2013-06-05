#include "interrupt_handler.h"
#include "icu.h"
#include "bwio.h"
#include "timer.h"

void init_interrupts() {
  clear_timer_interrupt();

  enable_interrupt(SOFT_INTERRUPT);
  //enable_interrupt(TIMER_INTERRUPT);
  //int val = *(int *)(VIC2_BASE + INT_ENABLE_OFFSET);
  //*(int *)(VIC2_BASE + INT_ENABLE_OFFSET) = 0x80000;

  int i;
// 40-44
  for (i = 41; i < 63; i++) {
    enable_interrupt(i);
  }
}

void process_interrupt() {
  bwprintf(COM2, "HWI! Int: %x , %x \n",*(int *)(VIC1_BASE), *(int *)VIC2_BASE);
  clear_soft_int();
  clear_timer_interrupt();
}

void await_event(Task* task, int event) {

}

