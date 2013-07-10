#include "timings.h"
#include "timer.h"
#include "debug.h"
#include "ourio.h"

#define TASK_BASE 0
#define MAX_PROCESSES (KERNEL + 1)

typedef struct timing_info {
  int total_edges;
  int last_edge_val;
} timing_info;

static timing_info timings[MAX_PROCESSES];
static int program_start_time;

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

void start_timing_task(Task* t) {
  start_timing(TASK_BASE + t->tid);
}

void end_timing_task(Task* t) {
  end_timing(TASK_BASE + t->tid);
}

void start_timing(int process) {
  ASSERT(timings[process].last_edge_val == 0, "timing.h: start_timing: process: %d", process);
  timings[process].last_edge_val = edges();
}

void end_timing(int process) {
  if (timings[process].last_edge_val != 0) {
    timings[process].total_edges += (edges() - timings[process].last_edge_val);
    timings[process].last_edge_val = 0;
  }
}

void print_timing_info(int process, int total_time) {
  timing_info* t_info = timings + process;
  if (t_info->total_edges != 0) {
    if (process >= 0 && process < MAX_TASKS) {
      printf(COM2, "Task %d: Execution Time = %dus, %d percent of total\n",
             process, edges_to_micros(t_info->total_edges), (100 * t_info->total_edges) / total_time);
    } else {
      printf(COM2, "Kernel: Execution Time = %dus, %d percent of total\n",
             edges_to_micros(t_info->total_edges), (100 * t_info->total_edges) / total_time);
    }
  }
}

void print_timings() {
  int total_execution_time = edges() - program_start_time;

  int i;
  for (i = 0; i < MAX_PROCESSES; i++) {
    print_timing_info(i, total_execution_time);
  }
}
