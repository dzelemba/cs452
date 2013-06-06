#include "clockserver.h"
#include "heap.h"
#include "syscall.h"
#include "task.h"
#include "priorities.h"
#include "icu.h"
#include "events.h"

#include "bwio.h"

#define CLOCKSERVER_PRIORITY MAX_PRI
#define NOTIFIER_PRIORITY MAX_PRI

// We can make the clockserver faster by never actually implementing the user-task.
// This is a lot more work, but we can get rid of 2-3 context switches every 10ms.

static heap listeners;
static unsigned int ticks;
static int clockserver_tid;

void notifier_run() {
  while (1) {
    AwaitEvent(EVENT_TIMER);
    Send(clockserver_tid, (char *)0, 0, (char *)0, 0);

    bwprintf(COM2, "Notifier Run.\n");
  }

  Exit();
}

void clockserver_run() {
  init_heap(&listeners);
  ticks = 0;

  int notifier_tid = Create(MAX_PRI, &notifier_run);

  int tid, wakeup_time;
  while (1) {
    Receive(&tid, (char *)&wakeup_time, sizeof(int));
    if (tid == notifier_tid) {
      ticks++;
      while (heap_min_pri(&listeners) <= ticks) {
        int waking_tid = (int) heap_delete_min(&listeners);
        Reply(waking_tid, (void *)0, 0);
      }
    } else {
      heap_insert(&listeners, wakeup_time, (void *) tid);
    }
  }

  Exit();
}

/* Public Methods */

void start_clockserver() {
  clockserver_tid = Create(CLOCKSERVER_PRIORITY, &clockserver_run);
}

int Delay(int t) {
  if (t <= 0) {
    Pass(); // OPTIONAL
  } else {
    int wakeup_time = ticks + t;
    Send(clockserver_tid, (char *)&wakeup_time, sizeof(int), (void *)0, 0);
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
    Send(clockserver_tid, (char *)&wakeup_time, sizeof(int), (void *)0, 0);
  }
  return 0;
}
