#include "all_tests.h"
#include <bwio.h>
#include "kernel.h"
#include "scheduler.h"
#include "syscall.h"
#include "task.h"
#include "test_helpers.h"

#define TASKS_TO_CREATE (MAX_TASKS - 2)

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
  for (i = 0; i < TASKS_TO_CREATE; i++) {
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

  assert_int_equals(TASKS_TO_CREATE + 1, tasks_run, "Task Creation Errors: Tasks Didn't Run");

  if (did_fail()) {
    bwprintf(COM2, "Task Creation Errors Test Failed!\n");
  } else {
    bwprintf(COM2, "Task Creation Errors Test Passed!\n");
  }
}
