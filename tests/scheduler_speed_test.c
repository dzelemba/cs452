#include <bwio.h>
#include <ts7200.h>

#include "all_tests.h"
#include "kernel.h"
#include "syscall.h"
#include "test_helpers.h"
#include "timer.h"
#include "priorities.h"

#define ITERATIONS 1000

/* Timing get_next_task */

static void time_pass(char* name) {
  int i;
  unsigned int t1 = ticks();
  for (i = 0; i < ITERATIONS; i++) {
    Pass();
  }
  unsigned int t2 = ticks();
  bwprintf(COM2, "%s Test: Average time for Pass: %d\n", name,
                 ticks_to_micros((t2 - t1) / ITERATIONS));
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

static int total_ticks;

static void time_adding_all_tasks() {
  int i;
  unsigned int t1, t2;

  // Don't add a task of MAX_PRI so no other tasks run during this test.
  // Don't add a task of MIN or VLOW pri so all the tasks get cleaned up before
  // the next run.
  t1 = ticks();
  for (i = VLOW_PRI_1; i > MAX_PRI; i--) {
    Create(i, &dummy);
  }
  t2 = ticks();
  total_ticks += (t2 - t1) / (NUM_PRIORITY_TYPES - 3);

  Exit();
}

static void time_add_task() {
  int i;
  total_ticks = 0;

  for (i = 0; i < ADD_TASK_ITERATIONS; i++) {
    Create(MAX_PRI, &time_adding_all_tasks);
  }
  bwprintf(COM2, "Add Task Test: Average time to create task: %d\n",
                 ticks_to_micros(total_ticks / (ADD_TASK_ITERATIONS)));

  Exit();
}

static void first() {
  Create(VLOW_PRI, &low_pri_test);
  Create(MAX_PRI, &hi_pri_test);
  Create(VLOW_PRI, &time_add_task);

  Exit();
}

void run_scheduler_speed_test() {
  init_kernel();
  reset_did_fail();

  kernel_add_task(MIN_PRI, &first);

  kernel_run();
  if (did_fail()) {
    bwprintf(COM2, "Syscall Speed Test Failed!\n");
  } else {
    bwprintf(COM2, "Syscall Speed Test Passed!\n");
  }
}
