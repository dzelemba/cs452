#ifndef __TIMINGS_H_
#define __TIMINGS_H_

#include "task.h"
#include "syscall.h"

typedef enum timed_process {
  TASK_BASE,
  SYSCALL_BASE = MAX_TASKS + 1,
  KERNEL = SYSCALL_BASE + NUM_SYSCALLS + 1,
  KERNEL_MEMCPY,
  SCHEDULER,
  MAX_PROCESSES
} timed_process;

void init_timings();

void start_timing_syscall(int syscall);

void end_timing_syscall(int syscall);

void start_timing_task(Task* t);

void end_timing_task(Task* t);

void start_timing(timed_process p);

void end_timing(timed_process p);

void print_timings();

#endif

