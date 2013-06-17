#ifndef __SIMPLE_SM_H__
#define __SIMPLE_SM_H__

/*
 * A simple state machine that waits for a certain number of events.
 */

typedef struct simple_sm {
  int* events;
  int numEvents;
} simple_sm;

void sm_create(simple_sm* sm, int numEvents);

void sm_report_event(simple_sm* sm, int i);

int sm_is_ready(simple_sm* sm);

void sm_reset(simple_sm* sm);

#endif
