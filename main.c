#include "syscall.h"
#include <bwio.h>
#include "task.h"
#include "context_switch.h"
#include "scheduler.h"

void user_task2() {
  bwprintf(COM2, "User task #2!\n");

  int tid = MyTid();
  bwprintf(COM2, "User task #2 tid: %d\n", tid);

  int p_tid = MyParentTid();
  bwprintf(COM2, "User task #2 parent tid: %d\n", p_tid);

  Exit();
}

void user_task() {
  bwprintf(COM2, "User task #1!\n");

  int tid = MyTid();
  bwprintf(COM2, "User task #1 tid: %d\n", tid);

  int child_tid = Create(1, &user_task2);
  bwprintf(COM2, "User task #1! Child tid: %d\n", child_tid);

  int p_tid = MyParentTid();
  bwprintf(COM2, "User task parent tid: %d\n", p_tid);

  Pass();
  bwprintf(COM2, "User task #1! 5\n");

  Exit();
}

int process_request(Task* task, Request* request) {
  Task* new_task;
  switch (request->syscall) {
    case 0: /* Create */
      new_task = task_create(task->tid, (int)request->args[0], (void *)request->args[1]);
      if (new_task) {  
        if (scheduler_add_task(request->args[0] /* Priority */, new_task)) {
          return -1; // Invalid priority.
        }
        return new_task->tid;
      } else {
        return -2; // Kernel out of task descriptors
      }
    case 1: /* MyTid */
      return task->tid;
    case 2: /* ParentTid */
      return task->parent_tid;
    case 3: /* Pass */
      scheduler_move_to_back(task->priority);
      break;
    case 4: /* Exit */
      scheduler_remove_task(task->priority);
      task_delete(task->tid);
      break;
    default:
      bwprintf(COM2, "Illegal syscall number: %d\n", request->syscall);
  }

  return 0;
}

int main(int argc, char** argv) {
  // First set up the kernel. 
  *(int *)(0x28) = (int)&k_enter; 
  init_tasks();
  scheduler_init();

  Task* first_task = task_create(-1 /* Parent tid */, 1 /* Priority */, &user_task);
  scheduler_add_task(1 /* Priority */, first_task);
  
  Task* next_task;
  int user_retval = 0;
  while (1) {
    next_task = scheduler_get_next_task();
    if (next_task == 0) {
      break;
    }

    Request* request = k_exit(user_retval, &next_task->stack_position);
    user_retval = process_request(next_task, request);
  }

  bwprintf(COM2, "Main Exiting... \n");

  return 0;
}
