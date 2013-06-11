#include "icu.h"
#include "bwio.h"

static void set_at_offset(int offset, int interrupt) {
  if (interrupt < 32) {
    *(int *)(VIC1_BASE + offset) |= (1 << interrupt);
  } else if (interrupt < 64) {
    *(int *)(VIC2_BASE + offset) |= (1 << (interrupt - 32));
  } else {
    bwprintf(COM2, "set_at_offset: Invalid interrupt number\n");
  }
}

static int get_at_offset(int offset, int interrupt) {
  if (interrupt < 32) {
    return (*(int *)(VIC1_BASE + offset) & (1 << interrupt)) != 0;
  } else if (interrupt < 64) {
    return (*(int *)(VIC2_BASE + offset) & (1 << (interrupt - 32))) != 0;
  } else {
    bwprintf(COM2, "get_at_offset: Invalid interrupt number\n");
    return 0;
  }
}

void clear_soft_int() {
  *(int *)(VIC1_BASE + SOFT_INT_CLEAR_OFFSET) = 1;
}

void trigger_interrupt(int interrupt) {
  set_at_offset(SOFT_INT_OFFSET, interrupt);
}

void enable_interrupt(int interrupt) {
  set_at_offset(INT_ENABLE_OFFSET, interrupt);
}

void disable_interrupt(int interrupt) {
  set_at_offset(INT_ENABLE_CLEAR_OFFSET, interrupt);
}

int check_interrupt(int interrupt) {
  return get_at_offset(IRQ_STATUS_OFFSET, interrupt);
}
