#include "clockserver.h"
#include "events.h"
#include "heap.h"
#include "icu.h"
#include "ourlib.h"
#include "priorities.h"
#include "syscall.h"
#include "task.h"

// We can make the clockserver faster by never actually implementing the user-task.
// This is a lot more work, but we can get rid of 2-3 context switches every 10ms.

static heap_node listeners_mem[MAX_TASKS];
static heap listeners;
static int ticks;
static int clockserver_tid;

void notifier_run() {
  while (1) {
    AwaitEvent(EVENT_TIMER);
    Send(clockserver_tid, (char *)0, 0, (char *)0, 0);
  }

  Exit();
}

void clockserver_run() {
  // TODO(dzelemba): Figure out why this causes the program to hang.
  //RegisterAs("Clockserver");
  init_heap(&listeners, listeners_mem, MAX_TASKS);
  ticks = 0;

  int notifier_tid = Create(MAX_PRI, &notifier_run);

  int tid, wakeup_time;
  while (1) {
    Receive(&tid, (char *)&wakeup_time, sizeof(int));
    if (tid == notifier_tid) {
      ticks++;
      while (listeners.size > 0 && heap_min_pri(&listeners) <= ticks) {
        int waking_tid = (int) heap_delete_min(&listeners);
        Reply(waking_tid, NULL, 0);
      }
      Reply(notifier_tid, NULL, 0);
    } else {
      heap_insert(&listeners, wakeup_time, (void *) tid);
    }
  }

  Exit();
}

/* Public Methods */

void start_clockserver() {
  clockserver_tid = Create(MAX_PRI, &clockserver_run);
}

int Delay(int t) {
  if (t <= 0) {
    Pass(); // OPTIONAL
  } else {
    int wakeup_time = ticks + t;
    Send(clockserver_tid, (char *)&wakeup_time, sizeof(int), NULL, 0);
  }
  return 0;
}

int Time() {
  return ticks;
}

int DelayUntil(int t) {
  if (t <= ticks) {
    Pass(); // OPTIONAL
  } else {
    int wakeup_time = t;
    Send(clockserver_tid, (char *)&wakeup_time, sizeof(int), NULL, 0);
  }
  return 0;
}
