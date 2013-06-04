#include "interrupt_handler.h"
#include "icu.h"
#include "bwio.h"

void init_interrupts() {
  // Turn on a single interrupt
  *(int *)(VIC1_BASE + INT_ENABLE_OFFSET) = 1;
}

void process_interrupt() {
  clear_soft_int();
  bwprintf(COM2, "HWI!\n");
}

void await_event(Task* task, int event) {

}

