#ifndef __ICU_H__
#define __ICU_H__

#define VIC1_BASE 0x800b0000
#define VIC2_BASE 0x800c0000

#define IRQ_STATUS_OFFSET 0x00
#define INT_SELECT_OFFSET 0x0c
#define INT_ENABLE_OFFSET 0x10
#define SOFT_INT_OFFSET   0x18
#define SOFT_INT_CLEAR_OFFSET 0x1c

void clear_soft_int();

void trigger_interrupt();

#endif
