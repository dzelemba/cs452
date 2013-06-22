#ifndef __TRAIN_H__
#define __TRAIN_H__

#define NUM_TRAINS 80
#define NUM_SWITCHES 22
#define NUM_SPEEDS 15

/*
 * Methods for controlling the trains.
 */

void init_trains();

/* Train Methods */

void tr_set_speed(int speed, int train);

void tr_reverse(int train);

/* Switch Methods */

void tr_sw(int switch_number, char switch_direction);

char get_switch_direction(int switch_number);

#endif
