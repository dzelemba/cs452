#ifndef __TASK_H__
#define __TASK_H__

#define STACK_SIZE (1024)

struct task {
  int tid;
  char stack[STACK_SIZE];

  void (*code);
};

#endif
