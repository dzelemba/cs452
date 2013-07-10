#include "location.h"
#include "physics.h"

void init_location(location* loc) {
  loc->train = 0;
  loc->node = 0;
  loc->cur_edge = 0;
  loc->um_past_node = 0;
  loc->d = FORWARD;
  loc->stopping_distance = DEFAULT_STOPPING_DISTANCE;
  loc->stopping_time = MAX_STOPPING_TIME;
  loc->prev_sensor_error = 0;
}
