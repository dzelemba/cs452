#include "clockserver.h"
#include "first_task.h"
#include "idle_task.h"
#include "ioserver.h"
#include "nameserver.h"
#include "syscall.h"
#include "train.h"

void first_user_task() {
  start_nameserver();
  start_clockserver();
  start_ioserver();
  start_idle_task();

  // Calls Putc so it needs to be here.
  init_trains();

  Exit();
}
