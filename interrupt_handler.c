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

  int i;
  for (i = 0; i < NUM_EVENTS; i++) {
    event_queues[i] = (Task *)0;
  }

  *(int *)(VIC2_BASE + INT_SELECT_OFFSET) = 0;
  *(int *)(VIC2_BASE + INT_ENABLE_OFFSET) = 0x80000;
  enable_interrupt(SOFT_INTERRUPT);
}

void clean_interrupts() {
  *(int *)(VIC2_BASE + INT_ENABLE_CLEAR_OFFSET) = 0x80000;
}

void process_interrupt() {
  if (((*(int *)VIC2_BASE) & 0x80000) == 0x80000) {
    Task* waiting_task = event_queues[EVENT_TIMER];
    if (waiting_task != (Task *)0) {
      task_set_state(waiting_task, READY);
      scheduler_add_task(waiting_task->priority, waiting_task);
      event_queues[EVENT_TIMER] = (Task *)0;
    }
    clear_timer_interrupt();
  } else {
    clear_soft_int();
  }

  /*bwprintf(COM2, "HWI! Int: %x , %x \n",*(int *)(VIC1_BASE), *(int *)VIC2_BASE);*/
  /*bwprintf(COM2, "HWI! Tix: %d\n", ticks());*/
}

void await_event(Task* task, int event) {
  ASSERT(event_queues[event] == (Task *)0, "await_event called on event with existing listener");
  event_queues[event] = task;

  task_set_state(task, EVENT_BLCK);
  scheduler_remove_task(task->priority);
}
