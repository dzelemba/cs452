#include "run_tests.h"
#include "all_tests.h"

void run_tests() {
  // This MUST run first because of assumptions about task ids.
  // TODO(dzelemba): Fix this.
  run_basic_test();
  run_multiple_priorities_test();
  run_context_switch_speed_test();
  run_task_creation_errors_test();
  run_message_passing_test();

  run_assignment_1_test();
}
