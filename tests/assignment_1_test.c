#include "all_tests.h"
#include "kernel.h"
#include <bwio.h>
#include "syscall.h"
#include "test_helpers.h"
#include "priorities.h"

static void second_user_task() {
  int tid = MyTid();
  int p_tid = MyParentTid();
  bwprintf(COM2, "MyTid: %d, MyParentTid: %d\n", tid, p_tid);
  Pass();
  bwprintf(COM2, "MyTid: %d, MyParentTid: %d\n", tid, p_tid);
  Exit();
}

static void first_user_task() {
  int tid = Create(LOW_PRI, &second_user_task);
  bwprintf(COM2, "Created: %d\n", tid);
  tid = Create(LOW_PRI, &second_user_task);
  bwprintf(COM2, "Created: %d\n", tid);
  tid = Create(HI_PRI, &second_user_task);
  bwprintf(COM2, "Created: %d\n", tid);
  tid = Create(HI_PRI, &second_user_task);
  bwprintf(COM2, "Created: %d\n", tid);

  bwprintf(COM2, "First: Exiting\n");
  Exit();
}

void run_assignment_1_test() {
  init_kernel();
  reset_did_fail();

  kernel_add_task(MED_PRI, &first_user_task);

  kernel_run();

  if (did_fail()) {
    bwprintf(COM2, "Assignment 1 Failed!\n");
  } else {
    bwprintf(COM2, "Assignment 1 Passed!\n");
  }
}
