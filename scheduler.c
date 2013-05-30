#include "scheduler.h"
#include "queue.h"
#include "bwio.h"
#include "debug.h"

#define NUM_PRIORITY_TYPES (4)

static queue task_queues[NUM_PRIORITY_TYPES];

// Wow. These API decisions are terrible.

void init_scheduler() {
  int i;
  for (i = 0; i < NUM_PRIORITY_TYPES; i++) {
    task_queues[i].start = 0;
    task_queues[i].end = 0;
  }
}

int is_valid_priority(int priority) {
  return priority >= 0 && priority < NUM_PRIORITY_TYPES;
}

int scheduler_add_task(int priority, Task* task) {
  METHOD_ENTRY("scheduler_add_task(%d, %x)\n", priority, (int) task);

  if (!is_valid_priority(priority)) {
    METHOD_EXIT("scheduler_add_task: invalid priority\n");
    return 1;
  }

  #ifdef DEBUG
  if (is_queue_full(&(task_queues[priority]))) {
    PRINT_DEBUG("EXCEPTION: task_queue[%d] is full\n", priority);
  }
  #endif

  push(&(task_queues[priority]), (int) task);

  METHOD_EXIT("scheduler_add_task: 0\n");
  return 0;
}

int scheduler_move_to_back(int priority) {
  if (!is_valid_priority(priority)) {
    return 1;
  }
  queue* q = &(task_queues[priority]);
  Task* task = (Task*) pop(q);
  push(q, (int) task);
  return 0;
}

int scheduler_remove_task(int priority) {
  if (!is_valid_priority(priority)) {
    return 1;
  }
  pop(&(task_queues[priority]));
  return 0;
}

Task* scheduler_get_next_task() {
  METHOD_ENTRY("scheduler_get_next_task\n");

  int i;
  for (i = 0; i < NUM_PRIORITY_TYPES; i++) {
    if (!is_queue_empty(&(task_queues[i]))) {
      Task* ret = (Task*) head(&(task_queues[i]));
      METHOD_EXIT("scheduler_get_next_task: %x\n", ret);
      return ret;
    }
  }

  METHOD_EXIT("scheduler_get_next_task: empty\n");
  return (Task*) 0;
}
