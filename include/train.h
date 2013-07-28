#ifndef __TRAIN_H__
#define __TRAIN_H__

#include "location.h"
#include "physics.h"

#define DEFAULT_TRAIN_SPEED 11

#define NUM_TRAINS 80
#define MAX_TRAINS 8
#define NUM_SPEEDS 15

/*
 * Note: When talking about wheels of the train we will use
 *   - Backward/Forward when referring to the direction the
 *     train is moving.
 *   - Back/Front when referring to the side of the train the
 *     pickup is on.
 */

#define PICKUP_TO_FRONTGRILL 30
#define PICKUP_TO_FRONTWHEEL 0
#define PICKUP_TO_BACKWHEEL 120
#define TRAIN_LENGTH 215

#define PICKUP_LENGTH_MM 50
#define PICKUP_LENGTH_UM PICKUP_LENGTH_MM * UM_PER_MM
#define PICKUP_LENGTH PICKUP_LENGTH_MM // Backwards compatibility

typedef struct train_array {
  int trains[MAX_TRAINS];
  int size;
} train_array;

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

void tr_get_done_trains(train_array* tr_array);

#endif
