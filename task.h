#ifndef __TASK_H__
#define __TASK_H__

#define STACK_SIZE (1024)
#define MAX_TASKS 127

#define UNUSED -1
#define READY 0
#define SEND_BLCK 1
#define RECV_BLCK 2
#define REPLY_BLCK 3
#define EVENT_BLCK 4
#define ZOMBIE 5

typedef struct Task {
  // Public
  int tid;
  int parent_tid;

  int priority;

  int state;

  // Return value to pass to the user program after a syscall.
  int retval;

  int* stack_position;
} Task;

void init_tasks();

Task* task_create(int parent_tid, int priority, void (*code));

Task* task_get(int tid);

void task_set_state(Task* task, int state);

void tid_set_state(int tid, int state);

int task_get_state(int tid);

// NOTE: This isn't fully working. We need to add a generation field to Task
// so that the same tid won't be given out to different tasks.
// This is OK for now since we don't actually use this.
void task_delete(int tid);

int get_num_user_tasks();

#endif
