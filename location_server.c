#include "location_server.h"
#include "syscall.h"
#include "sensor.h"
#include "sensor_server.h"
#include "track_node.h"
#include "ourlib.h"
#include "queue.h"
#include "track_data.h"
#include "priorities.h"
#include "train.h"
#include "debug.h"
#include "ourio.h"
#include "task.h"
#include "distance_server.h"

/*
 * Private Methods
 */

static int location_server_tid;

typedef enum location_server_message_type {
  TRACK_TRAIN,
  GET_UPDATES,
  SENSOR_UPDATE,
  DISTANCE_UPDATE
} location_server_message_type;

typedef struct location_server_message {
  location_server_message_type type;
  union {
    sensor_array sensors;
    location loc;
    struct {
      int train;
      int dx;
    } ds_update;
  };
} location_server_message;

void location_sensor_notifier() {
  location_server_message msg;
  msg.type = SENSOR_UPDATE;
  while (1) {
    msg.sensors.num_sensors = get_sensor_data(msg.sensors.sensors, MAX_NEW_SENSORS);
    Send(location_server_tid, (char *)&msg, sizeof(location_server_message), (void *)0, 0);
  }

  Exit();
}

void location_distance_notifier() {
  location_server_message msg;
  msg.type = DISTANCE_UPDATE;
  while (1) {
    ds_get_update(&msg.ds_update.train, &msg.ds_update.dx);
    Send(location_server_tid, (char *)&msg, sizeof(location_server_message), (void *)0, 0);
  }

  Exit();
}

typedef struct tracking_data {
  location* loc;
  sensor reverse_sensor;
  sensor next_possible_sensors[2];
  int num_next_possible_sensors;
} tracking_data;

typedef struct tracking_data_array {
  tracking_data t_data[MAX_TRAINS];
  int size;
} tracking_data_array;

void fill_next_possible_sensors(tracking_data* t_data) {
  track_node* node = t_data->loc->node;

  node2sensor(get_track(), node->reverse, &t_data->reverse_sensor);

  t_data->num_next_possible_sensors = 0;
  get_next_sensors(get_track(), node, t_data->next_possible_sensors, &t_data->num_next_possible_sensors);
}

void flip_direction(direction* d) {
  *d = (*d == FORWARD ? BACKWARD : FORWARD);
}

track_edge* get_next_edge(location* loc) {
  if (loc->node->type == NODE_BRANCH) {
    if (get_switch_direction(loc->node->num) == 'C') {
      return &loc->node->edge[DIR_CURVED];
    } else {
      return &loc->node->edge[DIR_STRAIGHT];
    }
  } else {
    return &loc->node->edge[DIR_AHEAD];
  }
}

void update_tracking_data_for_distance(location* loc) {
  if (loc->node->type == NODE_EXIT) {
    return;
  }

  track_edge* edge = get_next_edge(loc);
  if (loc->um_past_node / 1000 > edge->dist && edge->dest->type != NODE_SENSOR) {
    loc->um_past_node -= edge->dist * 1000;
    loc->node = edge->dest;
  }
}

int update_sensor_if_equal(location* loc, sensor* cur, sensor* new, int reversed) {
  if (sensor_equal(cur, new)) {
    track_edge* edge = get_next_edge(loc);
    loc->prev_sensor_error = loc->um_past_node / 1000 - edge->dist;
    loc->node = get_track_node(get_track(), sensor2idx(new->group, new->socket));
    loc->um_past_node = 0;
    if (reversed) {
      flip_direction(&loc->d);
    }
    return 1;
  }
  return 0;
}

void reply_to_tasks(queue* waiting_tasks, location_array* loc_array) {
  while (!is_queue_empty(waiting_tasks)) {
    Reply(pop(waiting_tasks), (char *)loc_array, sizeof(location_array));
  }
}

void location_server() {
  queue waiting_tasks;
  int q_mem2[MAX_TASKS];
  init_queue(&waiting_tasks, q_mem2, MAX_TASKS);
  q_set_name(&waiting_tasks, "Location Server Waiting Tasks");

  location_array loc_array;
  loc_array.size = 0;
  tracking_data_array t_data_array;
  t_data_array.size = 0;

  int tid, train, i, j, k, sensor_found, locations_changed;
  location* loc;
  location_server_message msg;

  while (1) {
    train = -1;
    Receive(&tid, (char *)&msg, sizeof(location_server_message));
    switch (msg.type) {
      case TRACK_TRAIN:
        Reply(tid, (void *)0, 0);
        ASSERT(loc_array.size < MAX_TRAINS, "location_server.c: track train request");

        // Notify distance server.
        ds_track_train(msg.loc.train);

        // Copy location data into location array
        memcpy((char *)&loc_array.locations[loc_array.size], (const char *)&msg.loc, sizeof(location));
        loc_array.size++;

        // Create tracking data.
        t_data_array.t_data[t_data_array.size].loc = &loc_array.locations[loc_array.size - 1];
        fill_next_possible_sensors(&t_data_array.t_data[t_data_array.size]);
        t_data_array.size++;

        reply_to_tasks(&waiting_tasks, &loc_array);
        break;
      case GET_UPDATES:
        push(&waiting_tasks, tid);
        break;
      case DISTANCE_UPDATE:
        Reply(tid, (void *)0, 0);
        loc = get_train_location(&loc_array, msg.ds_update.train);
        loc->um_past_node += msg.ds_update.dx;
        update_tracking_data_for_distance(get_train_location(&loc_array, msg.ds_update.train));
        reply_to_tasks(&waiting_tasks, &loc_array);
        break;
      case SENSOR_UPDATE: {
        Reply(tid, (void *)0, 0);
        locations_changed = 0;
        for (k = 0; k < t_data_array.size; k++) {
          sensor_found = 0;
          tracking_data* t_data = &t_data_array.t_data[k];
          for (i = 0; i < msg.sensors.num_sensors; i++) {
            // Check if we've reversed.
            // TODO(dzelemba): Sometimes the reverse switch gets fired when it shouldn't.
            // Find a way to handle this.
            sensor_found = update_sensor_if_equal(t_data->loc,
                                                  &t_data->reverse_sensor,
                                                  &msg.sensors.sensors[i],
                                                  1 /* Reversed */);
            if (!sensor_found) {
              // Check if we've hit one of the next possible sensors.
              for (j = 0; j < t_data->num_next_possible_sensors; j++) {
                if (update_sensor_if_equal(t_data->loc,
                                           &t_data->next_possible_sensors[j],
                                           &msg.sensors.sensors[i],
                                           0  /* Reversed */)) {
                  sensor_found = 1;
                }
              }
            }
            // Update the next sensors we're looking for and send out updates.
            if (sensor_found) {
              locations_changed = 1;
              fill_next_possible_sensors(t_data);
              break;
            }
          }
        }

        // Send out updates if locations have changed.
        if (locations_changed) {
          reply_to_tasks(&waiting_tasks, &loc_array);
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
  ERROR("train.c: get_train_location: Not tracking train %d\n", train);
  return 0;
}

void start_location_server() {
  location_server_tid = Create(MED_PRI, &location_server);
  Create(MED_PRI, &location_sensor_notifier);
  Create(MED_PRI, &location_distance_notifier);
}

void track_train(int train, location* loc) {
  location_server_message msg;
  msg.type = TRACK_TRAIN;
  loc->train = train;
  loc->um_past_node = 0;
  loc->prev_sensor_error = 0;
  memcpy((char *)&msg.loc, (const char*)loc, sizeof(location));

  Send(location_server_tid, (char *)&msg, sizeof(location_server_message), (void *)0, 0);
}

void get_location_updates(location_array* loc_array) {
  location_server_message msg;
  msg.type = GET_UPDATES;

  Send(location_server_tid, (char *)&msg, sizeof(location_server_message),
                            (char *)loc_array, sizeof(location_array));
}

char* direction_to_string(direction d) {
  switch (d) {
    case FORWARD:
      return "F";
    case BACKWARD:
      return "B";
  }
  return "UNKNOWN";
}