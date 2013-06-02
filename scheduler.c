#include "scheduler.h"
#include "queue.h"
#include "bwio.h"
#include "debug.h"
#include "priorities.h"
#include "linked_array.h"

static queue task_queues[NUM_PRIORITY_TYPES];
static linked_array ready_queues;

// Wow. These API decisions are terrible.

void init_scheduler() {
  int i;
  for (i = 0; i < NUM_PRIORITY_TYPES; i++) {
    init_queue(&task_queues[i]);
  }

  la_create(&ready_queues, NUM_PRIORITY_TYPES);
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

  queue* q = &task_queues[priority];
  if (is_queue_empty(q)) {
    la_insert(&ready_queues, priority, (void *)q);
  }
  push(q, (int) task);

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
  queue* q = &task_queues[priority];
  pop(q);
  if (is_queue_empty(q)) {
    la_remove(&ready_queues, priority);
  }
  return 0;
}

Task* scheduler_get_next_task() {
  METHOD_ENTRY("scheduler_get_next_task\n");

  if (!la_is_empty(&ready_queues)) {
    Task* ret = (Task*)head(la_head(&ready_queues));
    METHOD_EXIT("scheduler_get_next_task: %x\n", ret);
    return ret;
  }

  METHOD_EXIT("scheduler_get_next_task: empty\n");
  return (Task*) 0;
}
