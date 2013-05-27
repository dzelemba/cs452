#include "kernel.h"
#include "task.h"
#include "context_switch.h"
#include "scheduler.h"
#include "syscall.h"
#include "messenger.h"
#include <bwio.h>

int process_request(Task* task, Request* request) {
  Task* new_task;
  switch (request->syscall) {
    case CALLID_CREATE:
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
    case CALLID_MYTID:
      return task->tid;
    case CALLID_MYPARENTTID:
      return task->parent_tid;
    case CALLID_PASS:
      break;
    case CALLID_EXIT:
      scheduler_remove_task(task->priority);
      task_set_state(task, ZOMBIE);
      break;
    case CALLID_SEND:
      // Can we do better than this?
      messenger_send(task->tid,
                     (int) request->args[0],
                     (char*) request->args[1],
                     (int) request->args[2],
                     (char*) request->args[3],
                     (int) request->args[4]);
      break;
    case CALLID_RECEIVE:
      messenger_receive(task->tid,
                        (int*) request->args[0],
                        (char*) request->args[1],
                        (int) request->args[2]);
      break;
    case CALLID_REPLY:
      messenger_reply((int) request->args[0],
                      (char*) request->args[1],
                      (int) request->args[2]);
      break;
    default:
      bwprintf(COM2, "Illegal syscall number: %d\n", request->syscall);
  }

  return 0;
}

void init_kernel() {
  *(int *)(0x28) = (int)&k_enter;
  init_tasks();
  init_scheduler();
  init_messenger();
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
    // TODO: This is pretty hacky. Some syscalls will remove the current task from the
    // scheduler (i.e Exit, Send), so we should only move_to_back if that hasn't happened.
    if (next_task == scheduler_get_next_task()) {
      scheduler_move_to_back(next_task->priority);
    }
  }
}
