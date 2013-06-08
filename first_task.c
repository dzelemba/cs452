#include "first_task.h"
#include "nameserver.h"
#include "clockserver.h"
#include "idle_task.h"
#include "syscall.h"

void first_user_task() {
  start_nameserver();
  start_clockserver();
  start_idle_task();

  Exit();
}
