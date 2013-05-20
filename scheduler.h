#ifndef __SCHEDULER_H__
#define __SCHEDULER_H__

#include "task.h"

#define NUM_PRIORITY_TYPES (4)

void scheduler_add_task(int priority, Task* task);
void scheduler_move_to_back(int priority);
void scheduler_remove_task(int priority);

#endif
