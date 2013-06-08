#include "idle_task.h"
#include "priorities.h"
#include "syscall.h"

#include "bwio.h"

#define HALT_MODE_ADDR 0x80930008

void idle_task_run() {
  while (1) {
    int halting = *(int *)(HALT_MODE_ADDR);
    (void) halting;
  }

  Exit();
}

void start_idle_task() {
  Create(MIN_PRI, &idle_task_run);
}
