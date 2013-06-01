#include "first_task.h"
#include "nameserver.h"
#include "syscall.h"

void first_user_task() {
  start_nameserver();

  Exit();
}
