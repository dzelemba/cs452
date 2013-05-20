#include "scheduler.h"
#include "queue.h"

static queue task_queues[NUM_PRIORITY_TYPES];

// Wow. These API decisions are terrible.

void scheduler_add_task(int priority, Task* task) {
  push(&(task_queues[priority]), (int) task);
}

void scheduler_move_to_back(int priority) {
  queue* q = &(task_queues[priority]);
  Task* task = (Task*) pop(q);
  push(q, (int) task);
}

void scheduler_remove_task(int priority) {
  pop(&(task_queues[priority]));
}
