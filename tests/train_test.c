#include "all_tests.h"
#include "kernel.h"
#include "syscall.h"
#include "test_helpers.h"
#include "priorities.h"
#include "train.h"
#include "debug.h"
#include "ourlib.h"
#include "switch_server.h"

static void user_task() {
  USER_INFO("Train Test Starting...\n");
  tr_sw(10, 'S');
  tr_sw(10, 'C');

  tr_set_speed(7, 50);

  USER_INFO("Train Test Waiting...\n");
  Delay(500);

  USER_INFO("Train Test Reversing...\n");
  tr_reverse(50);

  Flush();
  Exit();
}

void run_train_test() {
  char* name = "Train Test";
  start_test(name);

  kernel_add_task(MED_PRI, &user_task);

  kernel_run();

  end_test(name);
}
