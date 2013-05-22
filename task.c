#include "task.h"
#include "queue.h"

static Task tasks[MAX_TASKS];
static queue free_task_ids;

Task* get_next_available_task() {
  if (is_queue_empty(&free_task_ids)) {
    return 0;
  }
  int task_id = pop(&free_task_ids);
  Task* task = &tasks[task_id];

  task->tid = task_id;
  return task;
}

void init_tasks() {
  init_queue(&free_task_ids);
  int i;
  for (i = 0; i < MAX_TASKS; i++) {
    push(&free_task_ids, i);
  }
}

Task* task_create(int parent_tid, int priority, void (*code)) {
  Task* task = get_next_available_task();
  if (task == 0) {
    return 0;
  }

  task->parent_tid = parent_tid;
  task->priority = priority;
  task->stack_position = task->stack + STACK_SIZE - 1;

  // Fill in stack with initial values.
  int* stack = task->stack_position; 

  // Positions 0 - 9 in the stack hold r4 -> r14 (minus r13, the stack ptr)
  // which don't need to be initialized.

  // Position 10 holds the spsr.
  int spsr = 0x10;
  *(stack - 10) = spsr;

  // Position 11 holds the pc which needs to be set to code
  *(stack - 11) = (int)code;

  // Increment stack ptr, making sure that it points to the last full byte.
  task->stack_position = stack - 11;

  return task;
}

void task_delete(int tid) {
  push(&free_task_ids, tid);
}
