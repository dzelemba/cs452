#include "all_tests.h"
#include "kernel.h"
#include <bwio.h>
#include "syscall.h"
#include "test_helpers.h"
#include "priorities.h"

static int flag;
static int did_run;

static int parent_tid;
static int child_1_tid;
static int child_2_tid;

static void user_task3() {
  child_2_tid = MyTid();

  int p_tid = MyParentTid();
  assert_int_equals(parent_tid, p_tid, "Basic Test: Child_2 Check Parent Tid");

  Exit();
}

static void user_task2() {
  int tid = MyTid();
  assert_int_equals(child_1_tid, tid, "Basic Test: Child Check Tid");

  int p_tid = MyParentTid();
  assert_int_equals(parent_tid, p_tid, "Basic Test: Child Check Parent Tid");

  flag = 1;

  Exit();
}

static void user_task() {
  did_run = 1;
  parent_tid = MyTid();

  child_1_tid = Create(MED_PRI, &user_task2);

  int p_tid = MyParentTid();
  assert_int_equals(-1, p_tid, "Basic Test: Parent Check Parent Tid");

  Pass();
  assert_int_equals(1, flag, "Basic Test: Parent Check Pass");

  // Let child run first to test that the right return value is returned.
  int tid = Create(HI_PRI, &user_task3);
  assert_int_equals(child_2_tid, tid, "Basic Test: Parent Check Child_2 Tid");

  Exit();
}

void run_basic_test() {
  init_kernel();
  reset_did_fail();
  flag = 0;
  did_run = 0;

  kernel_add_task(MED_PRI, &user_task);

  kernel_run();

  assert_int_equals(1, did_run, "Basic Test: User Program Never Ran");

  if (did_fail()) {
    bwprintf(COM2, "Basic Test Failed!\n");
  } else {
    bwprintf(COM2, "Basic Test Passed!\n");
  }
}
