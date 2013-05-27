#include "kernel.h"
#include "task.h"
#include "context_switch.h"
#include "scheduler.h"
#include "syscall.h"
#include <bwio.h>

int process_request(Task* task, Request* request) {
  Task* new_task;
  switch (request->syscall) {
    case 0: /* Create */
      // TODO: Move this into user space.
      if (!is_valid_priority((int)request->args[0])) {
        return -1; // Invalid priority.
      }

      new_task = task_create(task->tid, (int)request->args[0], (void *)request->args[1]);
      if (!new_task) {
        return -2; // Kernel out of task descriptors
      }

      scheduler_add_task(request->args[0] /* Priority */, new_task);
      return new_task->tid;
    case 1: /* MyTid */
      return task->tid;
    case 2: /* ParentTid */
      return task->parent_tid;
    case 3: /* Pass */
      break;
    case 4: /* Exit */
      scheduler_remove_task(task->priority);
      task_set_state(task, ZOMBIE);
      break;
    default:
      bwprintf(COM2, "Illegal syscall number: %d\n", request->syscall);
  }

  return 0;
}

void init_kernel() {
  *(int *)(0x28) = (int)&k_enter;
  init_tasks();
  scheduler_init();
}

void kernel_run() {
  Task* next_task;
  while (1) {
    next_task = scheduler_get_next_task();
    if (next_task == 0) {
      break;
    }

    Request* request = k_exit(next_task->retval, &next_task->stack_position);
    next_task->retval = process_request(next_task, request);

    // Must happen after process_request.
    scheduler_move_to_back(next_task->priority);
  }
}
