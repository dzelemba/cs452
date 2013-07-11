#include "timings.h"
#include "timer.h"
#include "debug.h"
#include "ourio.h"
#include "string.h"
#include "ourlib.h"
#include "nameserver.h"

typedef unsigned int uint_t;

typedef struct timing_info {
  uint_t total_edges;
  uint_t last_edge_val;
} timing_info;

static timing_info timings[MAX_PROCESSES];
static uint_t program_start_time;

void init_timings() {
  program_start_time = edges();

  int i;
  timing_info* t_info;
  for (i = 0; i < MAX_PROCESSES; i++) {
    t_info = timings + i;

    t_info->total_edges = 0;
    t_info->last_edge_val = 0;
  }
}

void start_timing_syscall(int syscall) {
  start_timing(SYSCALL_BASE + syscall);
}

void end_timing_syscall(int syscall) {
  end_timing(SYSCALL_BASE + syscall);
}

void start_timing_task(Task* t) {
  start_timing(TASK_BASE + t->tid);
}

void end_timing_task(Task* t) {
  end_timing(TASK_BASE + t->tid);
}

void start_timing(timed_process p) {
  ASSERT(timings[p].last_edge_val == 0, "timing.h: start_timing: process: %d", p);
  timings[p].last_edge_val = edges();
}

void end_timing(timed_process p) {
  if (timings[p].last_edge_val != 0) {
    timings[p].total_edges += (edges() - timings[p].last_edge_val);
    timings[p].last_edge_val = 0;
  }
}

#define MAX_NAME_SIZE 32

void timed_process_to_string(timed_process p, char* str) {
  if (p <= MAX_TASKS) {
    char* task_name = name_of(p);
    if (task_name != 0) {
      memcpy(str, task_name, MAX_NAME_SIZE);
      return;
    }

    string s;
    str_create(&s, str, MAX_NAME_SIZE);
    sprintf(&s, "Task %d", p);
  } else if (p <= SYSCALL_BASE + NUM_SYSCALLS) {
    string s;
    str_create(&s, str, MAX_NAME_SIZE);
    sprintf(&s, "Syscall %d", p - SYSCALL_BASE);
  } else  {
    switch (p) {
      case TASK_BASE:
      case SYSCALL_BASE:
        break;
      case KERNEL:
        memcpy(str, "KERNEL", MAX_NAME_SIZE);
        break;
      case KERNEL_MEMCPY:
        memcpy(str, "KERNEL_MEMCPY", MAX_NAME_SIZE);
        break;
      case SCHEDULER:
        memcpy(str, "SCHEDULER", MAX_NAME_SIZE);
        break;
      case MAX_PROCESSES:
        break;
    }
  }
}

void print_timing_info(timed_process p, uint_t total_time) {
  char str[MAX_NAME_SIZE];
  timed_process_to_string(p, str);

  timing_info* t_info = timings + p;
  if (t_info->total_edges != 0) {
    printf(COM2, "%s: Execution Time = %u edges, %u us, %u percent of total\n",
           str, t_info->total_edges, edges_to_micros(t_info->total_edges),
           (100 * t_info->total_edges) / total_time);
  }
}

void print_timings() {
  uint_t total_execution_time = edges() - program_start_time;

  printf(COM2, "Program Start Time: %u edges\n", program_start_time);
  printf(COM2, "Total Execution Time: %u edges, %ums\n",
         total_execution_time, edges_to_micros(total_execution_time));
  int i;
  for (i = 0; i < MAX_PROCESSES; i++) {
    print_timing_info(i, total_execution_time);
  }
}
