#ifndef __TASK_H__
#define __TASK_H__

#define STACK_SIZE (1024)

struct task {
  // Public
  int tid;

  // Private
  int stack[STACK_SIZE];
};

void task_create(struct task* task, int tid, void (*code));

// Just returns bottom of stack array.
int* task_get_stack(struct task* task);

#endif
