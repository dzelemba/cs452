#ifndef __TASK_H__
#define __TASK_H__

#define STACK_SIZE (1024)

typedef struct Task {
  // Public
  int tid;
  int parent_tid;

  int status;

  int* stack_position;

  // Private
  int stack[STACK_SIZE];
} Task;

void init_tasks();

Task* task_create(int parent_tid, void (*code));

#endif
