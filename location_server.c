#include "debug.h"
#include "distance_server.h"
#include "location_server.h"
#include "ourio.h"
#include "ourlib.h"
#include "physics.h"
#include "priorities.h"
#include "queue.h"
#include "sensor.h"
#include "sensor_server.h"
#include "syscall.h"
#include "task.h"
#include "track_data.h"
#include "track_node.h"
#include "track_node.h"
#include "train.h"
#include "switch_server.h"

#define NOT_ACCELERATING -1

/*
 * Private Methods
 */

static int location_server_tid;

typedef enum location_server_message_type {
  TRACK_TRAIN,
  GET_UPDATES,
  SENSOR_UPDATE,
  LS_TRAIN_REVERSE,
  LS_TRAIN_DIRECTION,
  DS_UPDATE_SPEED
} location_server_message_type;

typedef struct location_server_message {
  location_server_message_type type;
  union {
    sensor_array sensors;
    location loc;
    struct {
      int train;
      direction dir;
      int speed;
    } train_update;
  };
} location_server_message;

static int acceleration_start_time[MAX_TRAINS];
static unsigned int current_velocities[MAX_TRAINS];
static int current_speeds[MAX_TRAINS];
static int stopping_time[MAX_TRAINS];

void location_sensor_notifier() {
  location_server_message msg;
  msg.type = SENSOR_UPDATE;
  while (1) {
    msg.sensors.num_sensors = get_sensor_data(msg.sensors.sensors, MAX_NEW_SENSORS);
    Send(location_server_tid, (char *)&msg, sizeof(location_server_message), NULL, 0);
  }

  Exit();
}

void distance_notifier() {
  int now = Time();
  while (1) {
    // Use DelayUntil to reduce to constant factor drifting
    DelayUntil(now + 1);
    now++;
    Send(location_server_tid, NULL, 0, NULL, 0);
  }

  Exit();
}

typedef struct tracking_data {
  location* loc;
  track_node* next_sensor;
  track_node* missed_sensor;
  bool lost_train;
} tracking_data;

typedef struct tracking_data_array {
  tracking_data t_data[MAX_TRAINS];
  int size;
} tracking_data_array;

void flip_direction(direction* d) {
  *d = (*d == FORWARD ? BACKWARD : FORWARD);
}

/*
 * Here we switch merge switches to be in the direction we're coming from.
 * This is so that if we reverse on top of a merge, we don't derail.
 * It also helps the location_server get the proper direction on reverses.
 */
void perform_merge_action(track_edge* edge) {
  INFO(MISC, "Switched %s", edge->dest->name);
  track_node* br_node = edge->dest->reverse;
  if (&br_node->edge[DIR_STRAIGHT] == edge->reverse) {
    tr_sw(br_node->num, 'S');
  } else if (&br_node->edge[DIR_CURVED] == edge->reverse) {
    tr_sw(br_node->num, 'C');
  } else {
    INFO(MISC, "FAiled");
  }
}

void check_merge_action(tracking_data* t_data) {
  track_edge* next_edge = get_next_edge(t_data->loc->cur_edge->dest);
  if (next_edge != 0 && next_edge->dest->type == NODE_MERGE) {
    perform_merge_action(next_edge);
  }
}

void fill_in_tracking_data_with_edge(tracking_data* t_data) {
  t_data->next_sensor = get_next_sensor(t_data->loc->cur_edge);
}

void fill_in_tracking_data(tracking_data* t_data) {
  t_data->loc->cur_edge = get_next_edge(t_data->loc->node);
  fill_in_tracking_data_with_edge(t_data);
}

void increment_location(tracking_data* t_data) {
  ASSERT(t_data->loc->cur_edge != 0, "location_server.c: increment_location");

  t_data->loc->node = t_data->loc->cur_edge->dest;
  fill_in_tracking_data(t_data);
  check_merge_action(t_data);
}

void increment_location_to_sensor(tracking_data* t_data, track_node* sensor) {
  t_data->loc->node = sensor;
  fill_in_tracking_data(t_data);
  check_merge_action(t_data);
}

// If our model says we're this many mm past a sensor that we haven't hit,
// assume that it is broken.
#define BROKEN_SENSOR_ERROR 200

bool update_tracking_data_for_distance(tracking_data* t_data) {
  track_edge* edge = t_data->loc->cur_edge;
  if (t_data->lost_train || edge == NULL || t_data->loc->node->type == NODE_EXIT) {
    return false;
  }

  if (t_data->loc->um_past_node / UM_PER_MM > edge->dist) {
    if (edge->dest->type != NODE_SENSOR) {
      t_data->loc->um_past_node -= edge->dist * UM_PER_MM;
      increment_location(t_data);
      return true;
    }
  }

  // Check for broken sensor
  if (edge->dest->type == NODE_SENSOR) {
    int mm_past_sensor = t_data->loc->um_past_node / UM_PER_MM - edge->dist;
    if (mm_past_sensor > BROKEN_SENSOR_ERROR) {
      /*
       * If we've missed two sensors in a row, then report lost train
       * This can happen because either
       *   1) Our distances are way off, or
       *   2) The train is stuck
       * For 1), we should just wait until the train hits the sensor.
       * For 2), we should tell the distance server the train isn't
       * moving and then go into acceleration mode when it starts again.
       *
       * Unfortunately, there is no way to distinguish between the above cases,
       * so we'll assume 1) since its the more important case to get right.
       */
      if (t_data->missed_sensor != NULL) {
        INFO(LOCATION_SERVER,"Lost Train: %d at %s", t_data->loc->train, edge->dest->name);

        t_data->lost_train = true;

        // So we don't enter this case multiple times.
        t_data->loc->um_past_node = 0;
        return false;
      }
      INFO(LOCATION_SERVER,"Broken Sensor Found: %s", edge->dest->name);

      t_data->loc->um_past_node -= edge->dist * UM_PER_MM;
      t_data->missed_sensor = edge->dest;
      increment_location(t_data);
      return true;
    }
  }

  return false;
}

void update_tracking_data_for_sensor(tracking_data* t_data, sensor* s, bool* changed) {
  track_edge* edge = t_data->loc->cur_edge;
  track_node* sensor_node = get_track_node(sensor2idx(s->group, s->socket));

  int train_id = tr_num_to_idx(t_data->loc->train);
  bool is_stopping = acceleration_start_time[train_id] != NOT_ACCELERATING
                     && current_speeds[train_id] == 0;

  if (edge != NULL) {
    int prev_edge_distance = t_data->loc->cur_edge->dist * UM_PER_MM;

    if (t_data->next_sensor == sensor_node) {
      INFO(LOCATION_SERVER, "Updated to Next Sensor: %s", sensor_node->name);
      int expected_distance_to_sensor = get_next_sensor_distance(t_data->loc->node);
      t_data->loc->prev_sensor_error =
        t_data->loc->um_past_node / UM_PER_MM - expected_distance_to_sensor;
      increment_location_to_sensor(t_data, sensor_node);
    } else if (t_data->missed_sensor == sensor_node) {
      // Case where we updated our location prematurely
      INFO(LOCATION_SERVER, "Premature Update for Train: %d", t_data->loc->train);
      t_data->loc->node = sensor_node;
      t_data->loc->prev_sensor_error = t_data->loc->um_past_node / UM_PER_MM;
    } else {
      return;
    }

    if (t_data->lost_train) {
      INFO(LOCATION_SERVER,"Found Train: %d at %s", t_data->loc->train, sensor_node->name);
      t_data->lost_train = false;
    }

    // If we're stopping then don't reset distance so we get the final
    // stopping distance correct.
    if (is_stopping) {
      t_data->loc->um_past_node -= prev_edge_distance;
    } else {
      t_data->loc->um_past_node = 0;

    }
    t_data->missed_sensor = 0;
    *changed = true;
  }
}

void update_tracking_data_for_reverse(tracking_data* t_data, int train) {
  if (t_data->loc->cur_edge != 0) {
    // If the train is stopped on top of a sensor, it won't trigger the reverse sensor
    // so put it at the reverse sensor instead.
    if (t_data->loc->um_past_node <= PICKUP_LENGTH_UM) {
      t_data->loc->node = t_data->loc->node->reverse;
      t_data->loc->um_past_node = PICKUP_LENGTH * UM_PER_MM - t_data->loc->um_past_node;
      fill_in_tracking_data(t_data);
    } else {
      t_data->loc->node = t_data->loc->cur_edge->dest->reverse;
      t_data->loc->um_past_node =
        max(t_data->loc->cur_edge->dist * UM_PER_MM - t_data->loc->um_past_node + PICKUP_LENGTH_UM, 0);
      t_data->loc->cur_edge = t_data->loc->cur_edge->reverse;
      fill_in_tracking_data_with_edge(t_data);
    }
  } else {
    t_data->loc->node = t_data->loc->node->reverse;
    t_data->loc->um_past_node = 0;
    fill_in_tracking_data(t_data);
  }
  flip_direction(&t_data->loc->d);
}

void reply_to_tasks(queue* waiting_tasks, location_array* loc_array) {
  while (!is_queue_empty(waiting_tasks)) {
    Reply(pop(waiting_tasks), (char *)loc_array, sizeof(location_array));
  }
}

tracking_data* get_tracking_data(tracking_data_array* t_data_array, int train) {
  int i;
  for (i = 0; i < t_data_array->size; i++) {
    if (t_data_array->t_data[i].loc->train == train) {
      return &t_data_array->t_data[i];
    }
  }
  ERROR("location_server.c: tracking_data not found for train %d", train);
  return 0;
}

int get_distance_to_node(track_node* src, track_node* dest, int max_iters) {
  track_edge* next_edge = NULL;
  int i;
  for (i = 0; i < max_iters && src != dest; i++) {
    next_edge = get_next_edge(src);
    if (next_edge == NULL) {
      return -1;
    }
    src = next_edge->dest;
  }

  if (i == max_iters) {
    return -1;
  }

  return i;
}

#define LOOKAHEAD 3

tracking_data* attribute_sensor_to_train(tracking_data_array* t_array, sensor* s) {
  track_node* sensor_node = get_track_node(sensor2idx(s->group, s->socket));

  int minDist = LOOKAHEAD;
  tracking_data* attributed_train = NULL;

  int i;
  for (i = 0; i < t_array->size; i++) {
    int dist = get_distance_to_node(t_array->t_data[i].loc->node, sensor_node, LOOKAHEAD);
    if (dist != -1 && dist < minDist) {
      attributed_train = &t_array->t_data[i];
      minDist = dist;
    }
  }

  return attributed_train;
}

void location_server() {
  RegisterAs("Location Server");

  queue waiting_tasks;
  int q_mem2[MAX_TASKS];
  init_queue(&waiting_tasks, q_mem2, MAX_TASKS);
  q_set_name(&waiting_tasks, "Location Server Waiting Tasks");

  location_array loc_array;
  loc_array.size = 0;
  tracking_data_array t_data_array;
  t_data_array.size = 0;

  int tid, train, train_id, i, locations_changed, dt, dx, target_velocity;
  location* loc;
  location_server_message msg;

  int distance_notifier_tid = Create(MED_PRI, &distance_notifier);

  while (1) {
    Receive(&tid, (char *)&msg, sizeof(location_server_message));

    // Periodic distance update
    if (tid == distance_notifier_tid) {
      Reply(tid, NULL, 0);
      for (i = 0; i < loc_array.size; i++) {
        train = loc_array.locations[i].train;
        train_id = tr_num_to_idx(train);

        if (acceleration_start_time[train_id] != NOT_ACCELERATING) {
          if (current_speeds[train_id] > 0) {
            dt = Time() - acceleration_start_time[train_id];
            target_velocity = mean_velocity(train, current_speeds[train_id]);
            current_velocities[train_id] = accelerate(train, current_velocities[train_id], target_velocity, dt);
            if (current_velocities[train_id] == target_velocity) {
              acceleration_start_time[train_id] = NOT_ACCELERATING;
            }
          } else {
            // Stopping has no deceleration model
            if (stopping_time[train_id] - 1 == 0) {
              acceleration_start_time[train_id] = NOT_ACCELERATING;
              current_velocities[train_id] = 0;
            }
            stopping_time[train_id]--;
          }
        }

        if (acceleration_start_time[train_id] != NOT_ACCELERATING) {
          if (current_speeds[train_id] > 0) {
            dx = current_velocities[train_id] / NM_PER_UM;
          } else if (stopping_time[train_id] > 0) {
            dx = (MAX_STOPPING_TICKS * UM_PER_MM) / DEFAULT_STOPPING_TICKS;
          }
        } else {
          dx = current_velocities[train_id] / NM_PER_UM;
        }

        loc = get_train_location(&loc_array, train);
        loc->um_past_node += dx;

        bool has_new_location = update_tracking_data_for_distance(get_tracking_data(&t_data_array, train));
        if (acceleration_start_time[train_id] == NOT_ACCELERATING && has_new_location) {
          loc = get_train_location(&loc_array, train);
          // Exponential moving average
          int edge_velocity = (mean_velocity(train, current_speeds[train_id]) * piecewise_velocity(train, current_speeds[train_id], loc)) / 100;
          current_velocities[train_id] = (current_velocities[train_id] + edge_velocity) / 2;
        }
        loc->stopping_distance = stopping_distance(train, current_velocities[train_id]);
      }
      reply_to_tasks(&waiting_tasks, &loc_array);
    } else {
      switch (msg.type) {
        case TRACK_TRAIN:
          Reply(tid, NULL, 0);
          ASSERT(loc_array.size < MAX_TRAINS, "location_server.c: track train request");

          // Notify distance server.
          train_id = tr_num_to_idx(msg.loc.train);

          current_speeds[train_id] = 0;
          current_velocities[train_id] = 0;
          acceleration_start_time[train_id] = NOT_ACCELERATING;
          stopping_time[train_id] = 0;

          // Copy location data into location array
          memcpy((char *)&loc_array.locations[loc_array.size], (const char *)&msg.loc, sizeof(location));
          loc_array.size++;

          // Create tracking data.
          t_data_array.t_data[t_data_array.size].loc = &loc_array.locations[loc_array.size - 1];
          t_data_array.t_data[t_data_array.size].missed_sensor = NULL;
          t_data_array.t_data[t_data_array.size].lost_train = false;

          fill_in_tracking_data(&t_data_array.t_data[t_data_array.size]);
          t_data_array.size++;

          reply_to_tasks(&waiting_tasks, &loc_array);
          break;
        case GET_UPDATES:
          push(&waiting_tasks, tid);
          break;
        case LS_TRAIN_REVERSE:
          Reply(tid, NULL, 0);
          train = msg.train_update.train;
          update_tracking_data_for_reverse(get_tracking_data(&t_data_array, train), train);
          reply_to_tasks(&waiting_tasks, &loc_array);
          break;
        case LS_TRAIN_DIRECTION:
          Reply(tid, NULL, 0);
          train = msg.train_update.train;
          get_tracking_data(&t_data_array, train)->loc->d = msg.train_update.dir;

          reply_to_tasks(&waiting_tasks, &loc_array);
          break;
        case SENSOR_UPDATE: {
          Reply(tid, NULL, 0);
          locations_changed = 0; // Required so we don't send multiple updates for multiple sensor triggers
          for (i = 0; i < msg.sensors.num_sensors; i++) {
            tracking_data* attributed_train =
              attribute_sensor_to_train(&t_data_array, &msg.sensors.sensors[i]);
            if (attributed_train != NULL) {
              update_tracking_data_for_sensor(attributed_train, &msg.sensors.sensors[i],
                                              &locations_changed);
            }
          }
          if (locations_changed) {
            reply_to_tasks(&waiting_tasks, &loc_array);
          }
          break;
        }
        case DS_UPDATE_SPEED:
          Reply(tid, NULL, 0);
          train_id = tr_num_to_idx(msg.train_update.train);
          if (msg.train_update.speed == current_speeds[train_id]) {
            INFO(LOCATION_SERVER, "Ignored speed update: train %d to %d", msg.train_update.train, msg.train_update.speed);
            break;
          } else {
            INFO(LOCATION_SERVER, "Speed update: train %d (%d to %d)", msg.train_update.train,
                 current_speeds[train_id], msg.train_update.speed);
          }

          current_speeds[train_id] = msg.train_update.speed;
          if (current_speeds[train_id] == 0) {
            acceleration_start_time[train_id] = Time();
            // Division by 4 necessary to avoid integer overflow
            stopping_time[train_id] = (((MAX_STOPPING_TICKS / 4) * current_velocities[train_id]) / DEFAULT_NM_PER_TICK) * 4;
            INFO(LOCATION_SERVER, "Stopping time: train %d stops in %d ticks", msg.train_update.train, stopping_time[train_id]);
          } else {
            acceleration_start_time[train_id] = Time();
          }

          break;
      }
    }

  }
  Exit();
}

/*
 * Public Methods
 */

location* get_train_location(location_array* loc_array, int train) {
  int i;
  for (i = 0; i < loc_array->size; i++) {
    if (train == loc_array->locations[i].train) {
      return &loc_array->locations[i];
    }
  }
  /*ERROR("train.c: get_train_location: Not tracking train %d\n", train);*/
  return 0;
}

int get_track_index(track_node* track, location* loc) {
  return node2idx(track, loc->node);
}

void start_location_server() {
  location_server_tid = Create(MED_PRI, &location_server);
  Create(MED_PRI, &location_sensor_notifier);
}

void track_train(int train, location* loc) {
  location_server_message msg;
  msg.type = TRACK_TRAIN;
  loc->train = train;
  loc->um_past_node = (PICKUP_LENGTH + 10) * UM_PER_MM;
  loc->prev_sensor_error = 0;
  memcpy((char *)&msg.loc, (const char*)loc, sizeof(location));

  Send(location_server_tid, (char *)&msg, sizeof(location_server_message), NULL, 0);
}

void ls_set_direction(int train, direction dir) {
  location_server_message msg;
  msg.type = LS_TRAIN_DIRECTION;
  msg.train_update.train = train;
  msg.train_update.dir = dir;

  Send(location_server_tid, (char *)&msg, sizeof(location_server_message), NULL, 0);
}

void get_location_updates(location_array* loc_array) {
  location_server_message msg;
  msg.type = GET_UPDATES;

  Send(location_server_tid, (char *)&msg, sizeof(location_server_message),
                            (char *)loc_array, sizeof(location_array));
}

void ls_train_reversed(int train) {
  location_server_message msg;
  msg.type = LS_TRAIN_REVERSE;
  msg.train_update.train = train;
  Send(location_server_tid, (char *)&msg, sizeof(location_server_message), NULL, 0);
}

void ds_update_speed(int train, int speed) {
  location_server_message msg;
  msg.type = DS_UPDATE_SPEED;
  msg.train_update.train = train;
  msg.train_update.speed = speed;
  Send(location_server_tid, (char *)&msg, sizeof(location_server_message), NULL, 0);
}

char* direction_to_string(direction d) {
  switch (d) {
    case FORWARD:
      return "F";
    case BACKWARD:
      return "B";
  }
  return "U";
}

track_edge* get_next_edge(track_node* node) {
  if (node->type == NODE_BRANCH) {
    if (get_switch_direction(node->num) == 'C') {
      return &node->edge[DIR_CURVED];
    } else {
      return &node->edge[DIR_STRAIGHT];
    }
  } else if (node->type == NODE_EXIT) {
    return 0;
  } else {
    return &node->edge[DIR_AHEAD];
  }
}

track_node* get_next_sensor(track_edge* edge) {
  track_edge* next_edge = edge;
  while (next_edge != 0 && next_edge->dest->type != NODE_SENSOR) {
    next_edge = get_next_edge(next_edge->dest);
  }

  return next_edge == 0 ? 0 : next_edge->dest;
}

int get_next_sensor_distance(track_node* node) {
  track_edge* next_edge = get_next_edge(node);
  int dist = (next_edge == 0) ? -1 : next_edge->dist;
  while (next_edge != 0 && next_edge->dest->type != NODE_SENSOR) {
    next_edge = get_next_edge(next_edge->dest);
    dist += next_edge->dist;
  }

  return dist;
}
