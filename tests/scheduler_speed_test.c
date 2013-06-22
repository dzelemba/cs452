#include <ts7200.h>
#include "all_tests.h"
#include "kernel.h"
#include "syscall.h"
#include "test_helpers.h"
#include "timer.h"
#include "priorities.h"
#include "stdio.h"

#define ITERATIONS 1000

/* Timing get_next_task */

static void time_pass(char* name) {
  int i;
  unsigned int t1 = edges();
  for (i = 0; i < ITERATIONS; i++) {
    Pass();
  }
  unsigned int t2 = edges();
  printf(COM2, "%s Test: Average time for Pass: %d\n", name,
                 edges_to_micros((t2 - t1) / ITERATIONS));
  Flush();
}

static void low_pri_test() {
  time_pass("Low Pri");

  Exit();
}

static void hi_pri_test() {
  time_pass("Hi Pri");

  Exit();
}

/* Timing add_task */

#define ADD_TASK_ITERATIONS 2

static void dummy() {
  Exit();
}

static int total_edges;

static void time_adding_all_tasks() {
  int i, num_priorities_created;
  unsigned int t1, t2;

  num_priorities_created = 0;

  // Don't add a task of HI_PRI_1 so no other tasks run during this test.
  // Don't add a task of VLOW_PRI_1 so all the tasks get cleaned up before
  // the next run.
  t1 = edges();
  for (i = LOW_PRI; i > HI_PRI_1; i--) {
    if (!is_kernel_priority(i)) {
      num_priorities_created++;
      Create(i, &dummy);
    }
  }
  t2 = edges();
  total_edges += (t2 - t1) / num_priorities_created;

  Exit();
}

static void time_add_task() {
  int i;
  total_edges = 0;

  for (i = 0; i < ADD_TASK_ITERATIONS; i++) {
    Create(HI_PRI_1, &time_adding_all_tasks);
  }
  printf(COM2, "Add Task Test: Average time to create task: %d\n",
                 edges_to_micros(total_edges / (ADD_TASK_ITERATIONS)));

  Flush();
  Exit();
}

static void first() {
  // This task is VLOW_PRI, everything it spawns should be a higher so that they
  // are performed sequentially.
  Create(VLOW_PRI_1, &low_pri_test);
  Create(HI_PRI_1, &hi_pri_test);
  Create(VLOW_PRI_1, &time_add_task);

  Exit();
}

void run_scheduler_speed_test() {
  char* name = "Scheduler Speed Test";
  start_test(name);

  kernel_add_task(VLOW_PRI, &first);

  kernel_run();

  end_test(name);
}
