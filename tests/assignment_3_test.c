#include "all_tests.h"
#include "kernel.h"
#include "priorities.h"
#include "syscall.h"
#include "test_helpers.h"
#include "ourio.h"

static void priority_3() {
  int tid = MyTid();
  int delay_interval = 10;

  int i;
  for (i = 0; i < 20; i++) {
    Delay(delay_interval);
    printf(COM2, "tid: %d, delay interval: %d, # delay completed: %d\n", tid, delay_interval, (i + 1));
  }

  Exit();
}

static void priority_4() {
  int tid = MyTid();
  int delay_interval = 23;

  int i;
  for (i = 0; i < 9; i++) {
    Delay(delay_interval);
    printf(COM2, "tid: %d, delay interval: %d, # delay completed: %d\n", tid, delay_interval, (i + 1));
  }

  Exit();
}

static void priority_5() {
  int tid = MyTid();
  int delay_interval = 33;

  int i;
  for (i = 0; i < 6; i++) {
    Delay(delay_interval);
    printf(COM2, "tid: %d, delay interval: %d, # delay completed: %d\n", tid, delay_interval, (i + 1));
  }

  Exit();
}

static void priority_6() {
  int tid = MyTid();
  int delay_interval = 71;

  int i;
  for (i = 0; i < 3; i++) {
    Delay(delay_interval);
    printf(COM2, "tid: %d, delay interval: %d, # delay completed: %d\n", tid, delay_interval, (i + 1));
  }

  Exit();
}

static void first_user_task() {
  Create(HI_PRI_1, &priority_3);
  Create(HI_PRI, &priority_4);
  Create(MED_PRI_1, &priority_5);
  Create(MED_PRI, &priority_6);

  Exit();
}

void run_assignment_3_test() {
  char* name = "Assignment 3 Test";
  start_test(name);

  kernel_add_task(VLOW_PRI, &first_user_task);

  kernel_run();

  end_test(name);
}
