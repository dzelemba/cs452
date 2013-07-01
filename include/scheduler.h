#ifndef __SCHEDULER_H__
#define __SCHEDULER_H__

#include "task.h"

void init_scheduler();
int is_valid_priority(int priority);
int scheduler_add_task(int priority, Task* task);
int scheduler_move_to_back(int priority);
int scheduler_remove_task(int priority);
Task* scheduler_get_next_task();

#endif
