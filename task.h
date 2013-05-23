#ifndef __TASK_H__
#define __TASK_H__

#define STACK_SIZE (1024)
#define MAX_TASKS 1024

typedef struct Task {
  // Public
  int tid;
  int parent_tid;

  int priority;

  // Return value to pass to the user program after a syscall.
  int retval;

  int* stack_position;

  // Private
  int stack[STACK_SIZE];
} Task;

void init_tasks();

Task* task_create(int parent_tid, int priority, void (*code));

// NOTE: This isn't fully working. We need to add a generation field to Task
// so that the same tid won't be given out to different tasks.
// This is OK for now since we don't actually use this.
void task_delete(int tid);

#endif
