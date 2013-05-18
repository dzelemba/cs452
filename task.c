#include "task.h"

#define MAX_TASKS 1024

static Task tasks[MAX_TASKS];
static int next_task;

Task* get_next_available_task() {
  tasks[next_task].tid = next_task;
  return &tasks[next_task++];
}

void init_tasks() {
  next_task = 0;
}

// TODO: Return null if no more tids.
Task* task_create(int parent_tid, void (*code)) {
  Task* task = get_next_available_task();

  task->parent_tid = parent_tid;
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
