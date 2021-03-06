#include "all_tests.h"
#include "kernel.h"
#include "syscall.h"
#include "task.h"
#include "test_helpers.h"
#include "priorities.h"

#define TASKS_TO_CREATE (MAX_TASKS - 3)

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
  }
  retval = Create(LOW_PRI, &child_task);
  assert_int_equals(-2, retval, "Task Creation Errors: Too Many Tasks");

  Exit();
}

void run_task_creation_errors_test() {
  char* name = "Task Creation Errors Test";
  start_test(name);
  tasks_run = 0;

  kernel_add_task(HI_PRI, &user_task);

  kernel_run();

  assert_int_equals(TASKS_TO_CREATE + 1, tasks_run, "Task Creation Errors: Tasks Didn't Run");

  end_test(name);
}
