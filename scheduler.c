#include "scheduler.h"
#include "queue.h"
#include "bwio.h"
#include "debug.h"
#include "priorities.h"
#include "bitmask.h"

static queue task_queues[NUM_PRIORITY_TYPES];
static bitmask bm;

// Wow. These API decisions are terrible.

void init_scheduler() {
  int i;
  for (i = 0; i < NUM_PRIORITY_TYPES; i++) {
    init_queue(&task_queues[i]);
  }

  bm_create(&bm);
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
    bm_set(&bm, priority + 1);
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
    bm_unset(&bm, priority + 1);
  }
  return 0;
}

Task* scheduler_get_next_task() {
  METHOD_ENTRY("scheduler_get_next_task\n");

  int hi_pri = bm_getlowbit(&bm);
  if (hi_pri != 0) {
    Task* ret = (Task *)head(&task_queues[hi_pri - 1]);
    METHOD_EXIT("scheduler_get_next_task: %x\n", ret);
    return ret;
  }

  METHOD_EXIT("scheduler_get_next_task: empty\n");
  return (Task*) 0;
}
