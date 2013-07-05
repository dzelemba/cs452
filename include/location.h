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
  int um_past_node;
  direction d;
  int prev_sensor_error;
} location;

#endif
