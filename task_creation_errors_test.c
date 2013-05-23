#include "all_tests.h"
#include "scheduler.h"
#include "kernel.h"
#include <bwio.h>
#include "syscall.h"
#include "test_helpers.h"

static int tasks_run;

static void child_task() {
  tasks_run++;

  Exit();
}

static void user_task() {
  tasks_run++;

  // Try creating task with invalid priority.
  int retval = Create(-1 /* Priority */, &child_task);
  assert_int_equals(-1, retval, "Task Creation Errors: Invalid Priority");

  // Create too many tasks.
  int i;
  for (i = 0; i < MAX_TASKS - 1; i++) {
    retval = Create(LOW_PRI, &child_task);
    assert_true(retval >= 0, "Task Creation Errors: Create returned unexpected error");
  }
  retval = Create(LOW_PRI, &child_task);
  assert_int_equals(-2, retval, "Task Creation Errors: Too Many Tasks");

  Exit();
}

void run_task_creation_errors_test() {
  init_kernel();
  reset_did_fail();
  tasks_run = 0;

  Task* first_task = task_create(-1 /* Parent tid */, HI_PRI, &user_task);
  scheduler_add_task(HI_PRI, first_task);

  kernel_run();

  assert_int_equals(MAX_TASKS, tasks_run, "Task Creation Errors: Tasks Didn't Run");

  if (did_fail()) {
    bwprintf(COM2, "Task Creation Errors Test Failed!\n");
  } else {
    bwprintf(COM2, "Task Creation Errors Test Passed!\n");
  }
}
