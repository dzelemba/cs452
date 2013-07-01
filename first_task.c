#include "clockserver.h"
#include "first_task.h"
#include "idle_task.h"
#include "ioserver.h"
#include "nameserver.h"
#include "syscall.h"
#include "train.h"
#include "project.h"

void first_user_task() {
  start_nameserver();
  start_clockserver();
  start_ioserver();
  start_idle_task();

#ifndef TEST
  // Delay allows the ioserver to flush previous sensor data.
  Delay(50);
#endif

#ifndef TEST
  run_project();
#endif

  Exit();
}
