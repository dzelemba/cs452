#include "bwio.h"
#include "debug.h"
#include "events.h"
#include "icu.h"
#include "interrupt_handler.h"
#include "queue.h"
#include "scheduler.h"
#include "task.h"
#include "timer.h"

static Task* event_queues[NUM_EVENTS];

void init_interrupts() {
  clear_timer_interrupt();
  clear_soft_int();

  int i;
  for (i = 0; i < NUM_EVENTS; i++) {
    event_queues[i] = (Task *)0;
  }

  *(int *)(VIC1_BASE + INT_SELECT_OFFSET) = 0;
  *(int *)(VIC2_BASE + INT_SELECT_OFFSET) = 0;
  enable_interrupt(INTERRUPT_TIMER);
  enable_interrupt(INTERRUPT_SOFT);
}

void reset_interrupts() {
  disable_interrupt(INTERRUPT_TIMER);
  disable_interrupt(INTERRUPT_SOFT);
}

void process_interrupt() {
  Task* waiting_task;

  if (check_interrupt(INTERRUPT_TIMER)) {
    waiting_task = event_queues[EVENT_TIMER];
    if (waiting_task != (Task *)0) {
      task_set_state(waiting_task, READY);
      scheduler_add_task(waiting_task->priority, waiting_task);
      event_queues[EVENT_TIMER] = (Task *)0;
    }
    clear_timer_interrupt();
  } else {
    clear_soft_int();
  }
}

void await_event(Task* task, int event) {
  ASSERT(event_queues[event] == (Task *)0, "await_event called on event with existing listener");
  event_queues[event] = task;

  task_set_state(task, EVENT_BLCK);
  scheduler_remove_task(task->priority);
}
