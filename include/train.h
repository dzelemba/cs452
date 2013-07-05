#ifndef __TRAIN_H__
#define __TRAIN_H__

#include "location.h"

#define NUM_TRAINS 80
#define MAX_TRAINS 8
#define NUM_SWITCHES 22
#define NUM_SPEEDS 15

/*
 * Methods for controlling the trains.
 */

void init_trains();

/* Train Methods */

int tr_num_to_idx(int train);
int tr_idx_to_num(int idx);

// Pre Condition: No other trains can be moving.
void tr_track(int train);

void tr_set_speed(int speed, int train);

void tr_reverse(int train);

void tr_set_route(int train, int speed, location* loc);

/* Switch Methods */

void tr_sw(int switch_number, char switch_direction);

char get_switch_direction(int switch_number);

#endif
