#include <bwio.h>
#include <ts7200.h>

#include "all_tests.h"
#include "kernel.h"
#include "scheduler.h"
#include "syscall.h"
#include "test_helpers.h"
#include "timer.h"

#define ITERATIONS 1000

static void user_time_pass() {

  int i;
  unsigned int t1 = ticks();
  for (i = 0; i < ITERATIONS; i++) {
    Pass();
  }
  unsigned int t2 = ticks();
  bwprintf(COM2, "pass ticks: %d\n", (t2 - t1));


  Exit();
}

static void do_nothing() {
  Exit();
}

static void user_time_create() {

  int i;
  unsigned int t1 = ticks();
  for (i = 0; i < ITERATIONS; i++) {
    Create(VLOW_PRI, &do_nothing);
  }
  unsigned int t2 = ticks();
  bwprintf(COM2, "create ticks: %d\n", (t2 - t1));

  Exit();
}

void run_syscall_speed_test() {
  init_kernel();
  reset_did_fail();

  Task* first_task = task_create(-1 /* Parent tid */, HI_PRI, &user_time_pass);
  Task* second_task = task_create(-1 /* Parent tid */, HI_PRI, &user_time_create);
  scheduler_add_task(HI_PRI, first_task);
  scheduler_add_task(HI_PRI, second_task);

  kernel_run();
  if (did_fail()) {
    bwprintf(COM2, "Syscall Speed Test Failed!\n");
  } else {
    bwprintf(COM2, "Syscall Speed Test Passed!\n");
  }
}
