#ifndef __LOCATION_H__
#define __LOCATION_H__

#include "track_node.h"

typedef enum direction {
  FORWARD,
  BACKWARD
} direction;

typedef struct location {
  int train;
  track_node* node;
  track_edge* cur_edge;
  int um_past_node;
  direction d;
  int stopping_distance;
  int stopping_time;
  int prev_sensor_error;
} location;

void init_location(location* loc);

#endif
