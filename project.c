#include "project.h"
#include "all_tests.h"
#include "kernel.h"
#include "priorities.h"
#include "stdio.h"
#include "stdlib.h"
#include "strings.h"
#include "syscall.h"
#include "task.h"
#include "test_helpers.h"
#include "train.h"
#include "sensor_server.h"
#include "user_prompt.h"


// SHELL TASK

void shell_task() {
  start_sensor_server();
  start_user_prompt();

  Exit();
}

/*
 * Public Methods
 */

void run_project() {
  Create(VLOW_PRI, &shell_task);
}
