#include "all_tests.h"
#include "kernel.h"
#include "syscall.h"
#include "test_helpers.h"
#include "priorities.h"
#include "ourio.h"

static void second_user_task() {
  int tid = MyTid();
  int p_tid = MyParentTid();
  printf(COM2, "MyTid: %d, MyParentTid: %d\n", tid, p_tid);
  Pass();
  printf(COM2, "MyTid: %d, MyParentTid: %d\n", tid, p_tid);
  Exit();
}

static void first_user_task() {
  int tid = Create(LOW_PRI, &second_user_task);
  printf(COM2, "Created: %d\n", tid);
  tid = Create(LOW_PRI, &second_user_task);
  printf(COM2, "Created: %d\n", tid);
  tid = Create(HI_PRI, &second_user_task);
  printf(COM2, "Created: %d\n", tid);
  tid = Create(HI_PRI, &second_user_task);
  printf(COM2, "Created: %d\n", tid);

  printf(COM2, "First: Exiting\n");
  Exit();
}

void run_assignment_1_test() {
  char* name = "Assignment 1 Test";
  start_test(name);

  kernel_add_task(MED_PRI, &first_user_task);

  kernel_run();

  end_test(name);
}
