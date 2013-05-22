#ifndef __SCHEDULER_H__
#define __SCHEDULER_H__

#include "task.h"

#define HI_PRI 0
#define MED_PRI 1
#define LOW_PRI 2
#define VLOW_PRI 3

void scheduler_init();
int scheduler_add_task(int priority, Task* task);
int scheduler_move_to_back(int priority);
int scheduler_remove_task(int priority);
Task* scheduler_get_next_task();

#endif
