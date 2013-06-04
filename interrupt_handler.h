#ifndef __INTERRUPT_HANDLER_H__
#define  __INTERRUPT_HANDLER_H__

#include "task.h"

void init_interrupts();

void process_interrupt();

void await_event(Task* task, int event);

#endif
