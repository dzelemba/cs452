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
#include "track_node.h"

/*
 * Private Methods
 */

static int location_server_tid;

typedef enum location_server_message_type {
  TRACK_TRAIN,
  GET_UPDATES,
  SENSOR_UPDATE,
  DISTANCE_UPDATE,
  LS_TRAIN_REVERSE
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
    int train;
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
  track_edge* cur_edge;
} tracking_data;

typedef struct tracking_data_array {
  tracking_data t_data[MAX_TRAINS];
  int size;
} tracking_data_array;

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

void fill_in_tracking_data(tracking_data* t_data) {
  t_data->cur_edge = get_next_edge(t_data->loc);
}

void increment_location(tracking_data* t_data) {
  t_data->loc->node = t_data->cur_edge->dest;
  fill_in_tracking_data(t_data);
}

void update_tracking_data_for_distance(tracking_data* t_data) {
  if (t_data->loc->node->type == NODE_EXIT) {
    return;
  }

  track_edge* edge = t_data->cur_edge;
  if (t_data->loc->um_past_node / 1000 > edge->dist && edge->dest->type != NODE_SENSOR) {
    t_data->loc->um_past_node -= edge->dist * 1000;
    increment_location(t_data);
  }
}

int update_tracking_data_for_sensor(tracking_data* t_data, sensor* s) {
  track_edge* edge = t_data->cur_edge;
  if (t_data->cur_edge->dest == get_track_node(get_track(), sensor2idx(s->group, s->socket))) {
    increment_location(t_data);
    t_data->loc->prev_sensor_error = t_data->loc->um_past_node / 1000 - edge->dist;
    t_data->loc->um_past_node = 0;
    return 1;
  }

  return 0;
}

void update_tracking_data_for_reverse(tracking_data* t_data, int train) {
  t_data->loc->node = t_data->cur_edge->dest->reverse;
  flip_direction(&t_data->loc->d);
  t_data->loc->um_past_node = t_data->cur_edge->dist - t_data->loc->um_past_node;
  fill_in_tracking_data(t_data);
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

void location_server() {
  queue waiting_tasks;
  int q_mem2[MAX_TASKS];
  init_queue(&waiting_tasks, q_mem2, MAX_TASKS);
  q_set_name(&waiting_tasks, "Location Server Waiting Tasks");

  location_array loc_array;
  loc_array.size = 0;
  tracking_data_array t_data_array;
  t_data_array.size = 0;

  int tid, train, i, k, locations_changed;
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
        fill_in_tracking_data(&t_data_array.t_data[t_data_array.size]);
        t_data_array.size++;

        reply_to_tasks(&waiting_tasks, &loc_array);
        break;
      case GET_UPDATES:
        push(&waiting_tasks, tid);
        break;
      case LS_TRAIN_REVERSE:
        Reply(tid, (void *)0, 0);
        update_tracking_data_for_reverse(get_tracking_data(&t_data_array, msg.train), msg.train);
        reply_to_tasks(&waiting_tasks, &loc_array);
        break;
      case DISTANCE_UPDATE:
        Reply(tid, (void *)0, 0);
        loc = get_train_location(&loc_array, msg.ds_update.train);
        loc->um_past_node += msg.ds_update.dx;
        update_tracking_data_for_distance(get_tracking_data(&t_data_array, msg.ds_update.train));
        reply_to_tasks(&waiting_tasks, &loc_array);
        break;
      case SENSOR_UPDATE: {
        Reply(tid, (void *)0, 0);
        locations_changed = 0; // Required so we don't send multiple updates for multiple sensor triggers
        for (k = 0; k < t_data_array.size; k++) {
          tracking_data* t_data = &t_data_array.t_data[k];
          for (i = 0; i < msg.sensors.num_sensors; i++) {
            locations_changed |= update_tracking_data_for_sensor(t_data, &msg.sensors.sensors[i]);
          }
        }
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

int get_track_index(track_node* track, location* loc) {
  return node2idx(track, loc->node);
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

void ls_train_reversed(int train) {
  location_server_message msg;
  msg.type = LS_TRAIN_REVERSE;
  msg.train = train;
  Send(location_server_tid, (char *)&msg, sizeof(location_server_message), (void *)0, 0);
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
