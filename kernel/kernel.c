#include "context_switch.h"
#include "first_task.h"
#include "interrupt_handler.h"
#include "kernel.h"
#include "messenger.h"
#include "priorities.h"
#include "scheduler.h"
#include "ourlib.h"
#include "syscall.h"
#include "task.h"
#include "timer.h"
#include "debug.h"
#include "train.h"
#include "timings.h"
#include "ourio.h"

static int shutdown;

int process_request(Task* task, Request* request) {
  if (request == 0) {
    process_interrupt();
    return 0;
  }

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
      messenger_incomplete(task->tid);
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
      return messenger_reply((int) request->args[0],
                             (char*) request->args[1],
                             (int) request->args[2]);
      break;
    case CALLID_AWAITEVENT:
      await_event(task, (int) request->args[0]);
      break;
    case CALLID_SHUTDOWN:
      shutdown = 1;
      break;
    default:
      ERROR("Illegal syscall number: %d\n", request->syscall);
  }

  return 0;
}

void init_cache() {
  // invalidate cache
  asm("mov r1, #0");
  asm("mcr p15, 0, r1, c7, c5, 0");

  // enable instruction cache
  asm("mrc p15, 0, r1, c1, c0, 0");
  asm("orr r1, r1, #4096"); // enable instruction cache
  asm("orr r1, r1, #4"); // enable data cache
  asm("mcr p15, 0, r1, c1, c0, 0");
}

#define DEVICE_CONFIG_ADDR 0x80930080

void init_kernel() {
  shutdown = 0;

  init_cache();
  *(int *)(0x28) = (int)&k_enter;
  *(int *)(0x38) = (int)&hwi_enter;

  int device_config = *(int *)(DEVICE_CONFIG_ADDR);
  *(int *)(DEVICE_CONFIG_ADDR) = device_config | 0x1;

  init_timings();
  init_stdlib();
  init_debug_timer();
  init_timer();
  init_tasks();
  init_scheduler();
  init_messenger();

  init_interrupts();

  // Create task that will intialize servers.
  kernel_add_task(MAX_PRI, &first_user_task);
}

void kernel_add_task(int priority, void* code) {
  Task* task = task_create(-1 /* Parent tid */, priority, code);
  scheduler_add_task(priority, task);
}

void kernel_run() {
  Task* next_task;
  while (1) {
    //start_timing(SCHEDULER);
    next_task = scheduler_get_next_task();
    //end_timing(SCHEDULER);
#ifdef TEST
    if (get_num_user_tasks() == 0) {
      break;
    }
#endif

    end_timing(KERNEL);
    start_timing_task(next_task);

    Request* request = k_exit(next_task->retval, &next_task->stack_position);

    end_timing_task(next_task);
    start_timing(KERNEL);

    // Map interrupt request to syscall number NUM_SYSCALLS
    //start_timing_syscall(request == 0 ? NUM_SYSCALLS : request->syscall);
    next_task->retval = process_request(next_task, request);
    //end_timing_syscall(request == 0 ? NUM_SYSCALLS : request->syscall);
    if (shutdown) {
      break;
    }

    // Must happen after process_request.
    // TODO: This is pretty hacky. Some syscalls will remove the current task from the
    // scheduler (i.e Exit, Send), so we should only move_to_back if that hasn't happened.
    //start_timing(SCHEDULER);
    if (next_task == scheduler_get_next_task()) {
      scheduler_move_to_back(next_task->priority);
    }
    //end_timing(SCHEDULER);

  }

  // Clear the screen.
  printf(COM2, "\033cn");
  printf(COM2, "\033[2J\n");
  print_timings();
  reset_interrupts();
}
