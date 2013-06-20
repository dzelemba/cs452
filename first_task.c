#include "clockserver.h"
#include "first_task.h"
#include "idle_task.h"
#include "ioserver.h"
#include "nameserver.h"
#include "syscall.h"

void first_user_task() {
  start_nameserver();
  start_clockserver();
  start_ioserver();
  start_idle_task();

  Exit();
}
