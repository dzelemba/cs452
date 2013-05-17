#include "task.h"

void task_create(struct task* task, int tid, void (*code)) {
  task->tid = tid;

  // Fill in stack with initial values.
  int* stack = task_get_stack(task);

  // Position 0 holds the pc which needs to be set to code
  *stack = (int)code;

  // Position 1 holds the spsr.
  int spsr = 0x10;
  *(stack - 1) = spsr;

  // Positions 2 - 12 in the stack hold r4 -> r14
  // which don't need to be initialized.
}

int* task_get_stack(struct task* task) {
  return task->stack + STACK_SIZE - 1;
}
