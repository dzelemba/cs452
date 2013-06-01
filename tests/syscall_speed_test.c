#include <bwio.h>
#include <ts7200.h>

#include "all_tests.h"
#include "kernel.h"
#include "syscall.h"
#include "test_helpers.h"
#include "timer.h"
#include "priorities.h"

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

  kernel_add_task(HI_PRI, &user_time_pass);
  kernel_add_task(HI_PRI, &user_time_create);

  kernel_run();
  if (did_fail()) {
    bwprintf(COM2, "Syscall Speed Test Failed!\n");
  } else {
    bwprintf(COM2, "Syscall Speed Test Passed!\n");
  }
}
