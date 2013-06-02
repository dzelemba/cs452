#include "all_tests.h"
#include "kernel.h"
#include <bwio.h>
#include "syscall.h"
#include "test_helpers.h"
#include "priorities.h"
#include "icu.h"

static void user_task() {
  bwprintf(COM2, "Before HWI\n");

  // Trigger HWI
  *(int *)(VIC1_BASE + SOFT_INT_OFFSET) = 1;

  bwprintf(COM2, "After HWI\n");

  Exit();
}

void run_hwi_test() {
  init_kernel();
  reset_did_fail();

  kernel_add_task(MED_PRI, &user_task);

  kernel_run();

  if (did_fail()) {
    bwprintf(COM2, "HWI Test Failed!\n");
  } else {
    bwprintf(COM2, "HWI Test Passed!\n");
  }
}
