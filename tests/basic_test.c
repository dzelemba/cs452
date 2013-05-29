#include "all_tests.h"
#include "scheduler.h"
#include "kernel.h"
#include <bwio.h>
#include "syscall.h"
#include "test_helpers.h"

static int flag;
static int did_run;

static void user_task3() {
  int tid = MyTid();
  assert_int_equals(3, tid, "Basic Test: Child_2 Check Tid");

  int p_tid = MyParentTid();
  assert_int_equals(1, p_tid, "Basic Test: Child_2 Check Parent Tid");

  Exit();
}

static void user_task2() {
  int tid = MyTid();
  assert_int_equals(2, tid, "Basic Test: Child Check Tid");

  int p_tid = MyParentTid();
  assert_int_equals(1, p_tid, "Basic Test: Child Check Parent Tid");

  flag = 1;

  Exit();
}

static void user_task() {
  did_run = 1;
  int tid = MyTid();
  assert_int_equals(1, tid, "Basic Test: Parent Check Tid");

  int child_tid = Create(MED_PRI, &user_task2);
  assert_int_equals(2, child_tid, "Basic Test: Parent Check Child Tid");

  int p_tid = MyParentTid();
  assert_int_equals(-1, p_tid, "Basic Test: Parent Check Parent Tid");

  Pass();
  assert_int_equals(1, flag, "Basic Test: Parent Check Pass");

  // Let child run first to test that the right return value is returned.
  int child_2_tid = Create(HI_PRI, &user_task3);
  assert_int_equals(3, child_2_tid, "Basic Test: Parent Check Child_2 Tid");

  Exit();
}

void run_basic_test() {
  init_kernel();
  reset_did_fail();
  flag = 0;
  did_run = 0;

  Task* first_task = task_create(-1 /* Parent tid */, MED_PRI, &user_task);
  scheduler_add_task(MED_PRI, first_task);

  kernel_run();

  assert_int_equals(1, did_run, "Basic Test: User Program Never Ran");

  if (did_fail()) {
    bwprintf(COM2, "Basic Test Failed!\n");
  } else {
    bwprintf(COM2, "Basic Test Passed!\n");
  }
}
