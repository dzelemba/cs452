#include "run_tests.h"
#include "all_tests.h"

void run_tests() {
  run_basic_test();
  run_multiple_priorities_test();
  run_task_creation_errors_test();
  run_message_passing_test();
  run_nameserver_test();

  // Performance Tests
  run_syscall_speed_test();
  run_srr_speed_test();
  run_scheduler_speed_test();

  //run_assignment_1_test();
  //run_rps_server_test();
}
