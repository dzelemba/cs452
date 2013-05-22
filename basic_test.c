#include "all_tests.h"
#include "scheduler.h"
#include "kernel.h"
#include <bwio.h>
#include "syscall.h"
#include "test_helpers.h"

static int flag;
static int did_run;

void user_task2() {
  int tid = MyTid();
  assert_int_equals(1, tid, "Basic Test: Child Check Tid");

  int p_tid = MyParentTid();
  assert_int_equals(0, p_tid, "Basic Test: Child Check Parent Tid");

  flag = 1;

  Exit();
}

void user_task() {
  did_run = 1;

  int tid = MyTid();
  assert_int_equals(0, tid, "Basic Test: Parent Check Tid");

  int child_tid = Create(1, &user_task2);
  assert_int_equals(1, child_tid, "Basic Test: Parent Check Child Tid");

  int p_tid = MyParentTid();
  assert_int_equals(-1, p_tid, "Basic Test: Parent Check Parent Tid");

  Pass();
  assert_int_equals(1, flag, "Basic Test: Parent Check Pass");

  Exit();
}

void run_basic_test() {
  reset_did_fail();
  flag = 0;
  did_run = 0;

  Task* first_task = task_create(-1 /* Parent tid */, 1 /* Priority */, &user_task);
  scheduler_add_task(1 /* Priority */, first_task);
  
  kernel_run();

  assert_int_equals(1, did_run, "Basic Test: User Program Never Ran");

  if (did_fail()) {
    bwprintf(COM2, "Basic Test Failed!\n"); 
  } else {
    bwprintf(COM2, "Basic Test Passed!\n"); 
  }
}
