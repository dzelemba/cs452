#include "interrupt_handler.h"
#include "icu.h"
#include "bwio.h"
#include "timer.h"

void init_interrupts() {
  clear_timer_interrupt();

  *(int *)(VIC2_BASE + INT_SELECT_OFFSET) = 0;
  *(int *)(VIC2_BASE + INT_ENABLE_OFFSET) = 0x80000;
  enable_interrupt(SOFT_INTERRUPT);
}

void clean_interrupts() {
  *(int *)(VIC2_BASE + INT_ENABLE_CLEAR_OFFSET) = 0x80000;
}

void process_interrupt() {
  bwprintf(COM2, "HWI! Int: %x , %x \n",*(int *)(VIC1_BASE), *(int *)VIC2_BASE);
  bwprintf(COM2, "HWI! Tix: %d\n", ticks());
  clear_timer_interrupt();
  clear_soft_int();
}

void await_event(Task* task, int event) {

}

