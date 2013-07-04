#ifndef __LOCATION_SERVER_H__
#define __LOCATION_SERVER_H__

#include "track_node.h"

// Max number of trains we are tracking.
#define MAX_TRAINS 2

typedef enum direction {
  FORWARD,
  BACKWARD
} direction;

typedef struct location {
  int train;
  track_node* node;
  int um_past_node;
  direction d;
  int prev_sensor_error;
} location;

typedef struct location_array {
  location locations[MAX_TRAINS];
  int size;
} location_array;

int get_track_index(track_node* track, location* loc);

location* get_train_location(location_array* loc_array, int train);

/*
 * Location Server
 */

void start_location_server();

void track_train(int train, location* loc);

void get_location_updates(location_array* loc_array);

void ls_train_reversed(int train);

char* direction_to_string(direction d);


#endif
