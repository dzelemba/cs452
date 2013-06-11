#ifndef __ICU_H__
#define __ICU_H__

#define VIC1_BASE 0x800b0000
#define VIC2_BASE 0x800c0000

#define IRQ_STATUS_OFFSET 0x00
#define INT_SELECT_OFFSET 0x0c
#define INT_ENABLE_OFFSET 0x10
#define INT_ENABLE_CLEAR_OFFSET 0x14
#define SOFT_INT_OFFSET   0x18
#define SOFT_INT_CLEAR_OFFSET 0x1c

// Unused so we can use it for testing.
#define INTERRUPT_SOFT 0
#define INTERRUPT_TIMER 51

void clear_soft_int();

void trigger_interrupt(int interrupt);

void enable_interrupt(int interrupt);

void disable_interrupt(int interrupt);

int check_interrupt(int interrupt);

#endif
