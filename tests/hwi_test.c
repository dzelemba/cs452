#include "all_tests.h"
#include "kernel.h"
#include <bwio.h>
#include "syscall.h"
#include "test_helpers.h"
#include "priorities.h"
#include "icu.h"

static void test_scratch_registers() {
  int a = 10;

  // Trigger HWI
  *(int *)(VIC1_BASE + SOFT_INT_OFFSET) = 1;

  // The compiler will fill in registers for this call before
  // executing the above instruction, so this tests that the scratch
  // registers are saved & restored.
  assert_int_equals(10, a, "HWI Interrupt Test Check Val 1");

  Exit();
}

static void user_task() {
  // Trigger HWI
  // Trigger within a function to test that the LR gets saved.
  trigger_interrupt(INTERRUPT_SOFT);

  trigger_interrupt(INTERRUPT_SOFT);

  trigger_interrupt(INTERRUPT_SOFT);

  Exit();
}

static void first() {
  Create(MED_PRI, &user_task);
  Create(MED_PRI, &test_scratch_registers);

  Exit();
}

void run_hwi_test() {
  init_kernel();
  reset_did_fail();

  kernel_add_task(MED_PRI, &first);

  kernel_run();

  if (did_fail()) {
    bwprintf(COM2, "HWI Test Failed!\n");
  } else {
    bwprintf(COM2, "HWI Test Passed!\n");
  }
}
