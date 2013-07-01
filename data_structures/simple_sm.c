#include "simple_sm.h"
#include "ourlib.h"
#include "debug.h"

void sm_create(simple_sm* sm, int numEvents) {
  sm->events = (int *)kmalloc(numEvents * sizeof(int));
  sm->numEvents = numEvents;

  sm_reset(sm);
}

void sm_report_event(simple_sm* sm, int i) {
  ASSERT(i >= 0 && i < sm->numEvents, "Simple State Machine: sm_report_event");

  sm->events[i] = 1;
}

int sm_is_ready(simple_sm* sm) {
  int i;
  for (i = 0; i < sm->numEvents; i++) {
    if (sm->events[i] == 0) {
      return 0;
    }
  }

  return 1;
}

void sm_reset(simple_sm* sm) {
  int i;
  for (i = 0; i < sm->numEvents; i++) {
    sm->events[i] = 0;
  }
}

