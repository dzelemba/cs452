#include "all_tests.h"
#include "kernel.h"
#include <bwio.h>
#include "syscall.h"
#include "test_helpers.h"
#include "priorities.h"

static int execution_order;

static void user_task_med() {
  assert_int_equals(5, execution_order, "Priorities Test: MED_PRI Task");
  execution_order++;

  Exit();
}

static void user_task_low() {
  assert_int_equals(3, execution_order, "Priorities Test: LOW_PRI Task");
  execution_order++;

  Exit();
}

static void user_task_vlow() {
  assert_int_equals(4, execution_order, "Priorities Test: VLOW_PRI Task");
  execution_order++;

  Create(MED_PRI, &user_task_med);

  assert_int_equals(6, execution_order, "Priorities Test: VLOW_PRI Task");
  execution_order++;

  Exit();
}

static void user_task_hi() {
  assert_int_equals(0, execution_order, "Priorities Test: HI_PRI Task");
  execution_order++;

  Create(VLOW_PRI, &user_task_vlow);

  assert_int_equals(1, execution_order, "Priorities Test: HI_PRI Task");
  execution_order++;

  Create(LOW_PRI, &user_task_low);

  assert_int_equals(2, execution_order, "Priorities Test: HI_PRI Task");
  execution_order++;

  Exit();
}

void run_multiple_priorities_test() {
  init_kernel();
  reset_did_fail();
  execution_order = 0;

  kernel_add_task(HI_PRI, &user_task_hi);

  kernel_run();

  if (did_fail()) {
    bwprintf(COM2, "Multiple Priorities Test Failed!\n");
  } else {
    bwprintf(COM2, "Multiple Priorities Test Passed!\n");
  }
}
