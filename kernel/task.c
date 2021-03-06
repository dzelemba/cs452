#include "priorities.h"
#include "queue.h"
#include "ourlib.h"
#include "task.h"

// Task ids need to be positive.
// Instead of dealing with adding/substracting 1 to go from task id
// to array index we will just not use position 0 in the array.
static Task tasks[MAX_TASKS + 1];
static int next_tid;
static int num_user_tasks;


Task* get_next_available_task() {
  if (next_tid > MAX_TASKS) {
    return 0;
  }

  int tid = next_tid;
  Task* task = &(tasks[tid]);
  task->tid = tid;
  next_tid++;

  return task;
}

void init_tasks() {
  next_tid = 1;
  num_user_tasks = 0;

  int i;
  for (i = 1; i < MAX_TASKS + 1; i++) {
    tasks[i].state = UNUSED;
  }
}

Task* task_create(int parent_tid, int priority, void (*code)) {
  Task* task = get_next_available_task();
  if (task == 0) {
    return 0;
  }

  if (!is_kernel_priority(priority)) {
    num_user_tasks++;
  }

  task->parent_tid = parent_tid;
  task->priority = priority;
  task->state = READY;
  task->stack_position = (int *)kmalloc(STACK_SIZE) + STACK_SIZE - 1;

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

Task* task_get(int tid) {
  return (tasks + tid);
}

void task_set_state(Task* task, int state) {
  task->state = state;
  if (!is_kernel_priority(task->priority) && state == ZOMBIE) {
    num_user_tasks--;
  }
}

void tid_set_state(int tid, int state) {
  task_set_state(task_get(tid), state);
}

int task_get_state(int tid) {
  return task_get(tid)->state;
}

int get_num_user_tasks() {
  return num_user_tasks;
}
