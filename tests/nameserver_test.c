#include "all_tests.h"
#include "scheduler.h"
#include "kernel.h"
#include <bwio.h>
#include "syscall.h"
#include "test_helpers.h"
#include "nameserver.h"

static int task_1_tid;
static int task_2_tid;
static int task_3_tid;
static int task_4_tid;

static int num_tasks_ran;

// Overwrite a name.
static void task_4() {
  num_tasks_ran++;

  assert_int_equals(0, RegisterAs("Task 3"), "Nameserver Test: Register Task 4");

  assert_int_equals(task_1_tid, WhoIs("Task 1"), "Nameserver Test: WhoIs Task 1");
  assert_int_equals(task_2_tid, WhoIs("Task 2"), "Nameserver Test: WhoIs Task 2");
  assert_int_equals(task_4_tid, WhoIs("Task 3"), "Nameserver Test: WhoIs Task 4");

  Exit();
}

static void task_3() {
  num_tasks_ran++;

  assert_int_equals(0, RegisterAs("Task 3"), "Nameserver Test: Register Task 3");

  assert_int_equals(task_1_tid, WhoIs("Task 1"), "Nameserver Test: WhoIs Task 1");
  assert_int_equals(task_2_tid, WhoIs("Task 2"), "Nameserver Test: WhoIs Task 2");
  assert_int_equals(task_3_tid, WhoIs("Task 3"), "Nameserver Test: WhoIs Task 3");

  task_4_tid = Create(MED_PRI, &task_4);

  Exit();
}

static void task_2() {
  num_tasks_ran++;

  assert_int_equals(0, RegisterAs("Task 2"), "Nameserver Test: Register Task 2");

  assert_int_equals(task_1_tid, WhoIs("Task 1"), "Nameserver Test: WhoIs Task 1");
  assert_int_equals(task_2_tid, WhoIs("Task 2"), "Nameserver Test: WhoIs Task 2");

  task_3_tid = Create(MED_PRI, &task_3);

  Exit();
}

static void task_1() {
  num_tasks_ran++;

  assert_int_equals(0, RegisterAs("Task 1"), "Nameserver Test: Register Task 1");

  assert_int_equals(task_1_tid, WhoIs("Task 1"), "Nameserver Test: WhoIs Task 1");

  task_2_tid = Create(MED_PRI, &task_2);

  Exit();
}

static void init_task() {
  assert_int_equals(-1, WhoIs("Task 1"), "Nameserver Test: WhoIs Task 1");

  task_1_tid = Create(MED_PRI, &task_1);

  Exit();
}

void run_nameserver_test() {
  init_kernel();
  reset_did_fail();
  num_tasks_ran = 0;

  Task* first_task = task_create(-1 /* Parent tid */, MED_PRI, &init_task);
  scheduler_add_task(MED_PRI, first_task);

  kernel_run();

  assert_int_equals(4, num_tasks_ran, "Nameserver Test: Not all tests ran");

  if (did_fail()) {
    bwprintf(COM2, "Nameserver Test Failed!\n");
  } else {
    bwprintf(COM2, "Nameserver Test Passed!\n");
  }
}
