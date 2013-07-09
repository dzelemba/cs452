#include "debug.h"
#include "distance_server.h"
#include "location_server.h"
#include "ourio.h"
#include "ourlib.h"
#include "physics.h"
#include "priorities.h"
#include "queue.h"
#include "syscall.h"
#include "task.h"
#include "track_data.h"
#include "train.h"

#define NOT_ACCELERATING -1

/*
 * Private Methods
 */

static int distance_server_tid;

static int acceleration_start_time[MAX_TRAINS];
static unsigned int current_velocities[MAX_TRAINS];
static unsigned int accumulated_dx[MAX_TRAINS];
static int current_speeds[MAX_TRAINS];

typedef enum distance_server_message_type {
  DS_TRACK_TRAIN,
  DS_UPDATE_SPEED,
  DS_GET_UPDATE,
  DS_NOTIFIER,
  DS_LOCATION_UPDATE
} distance_server_message_type;

// TODO: Messages now take up too much pointless space
typedef struct distance_server_message {
  distance_server_message_type type;
  int train;
  char speed;
  location_array loc_array;
} distance_server_message;

typedef struct distance_update_message {
  char train;
  int dx;
  int stopping_distance;
  int stopping_time;
} distance_update_message;

void distance_notifier() {
  // First we get what train we're a notifier for.
  int train, tid;
  Receive(&tid, (char *)&train, sizeof(int));
  Reply(tid, (void *)0, 0);

  // Now we do our job.
  distance_server_message msg;
  msg.type = DS_NOTIFIER;
  msg.train = train;

  int now = Time();
  while (1) {
    // Use DelayUntil to reduce to constant factor drifting
    DelayUntil(now + 1);
    now++;
    Send(distance_server_tid, (char *)&msg, sizeof(distance_server_message), (void *)0, 0);
  }

  Exit();
}

void distance_location_courier() {
  // Distance is based off location... which is based off distance. Huzzah engineering genius
  distance_server_message msg;
  msg.type = DS_LOCATION_UPDATE;

  while (1) {
    get_location_updates(&msg.loc_array);
    Send(distance_server_tid, (char *)&msg, sizeof(distance_server_message), (void *)0, 0);
  }

  Exit();
}

void distance_server() {
  int waiting_tid = 0;

  int notifier_tids[MAX_TRAINS];

  distance_update_message dum;
  distance_server_message msg;

  location_array train_locations;
  train_locations.size = 0;
  location* loc;

  int tid, train_id, dx, dt, target_velocity;

  while (1) {
    Receive(&tid, (char *)&msg, sizeof(distance_server_message));
    switch (msg.type) {
      case DS_TRACK_TRAIN:
        Reply(tid, (void *)0, 0);
        train_id = tr_num_to_idx(msg.train);

        current_speeds[train_id] = 0;
        accumulated_dx[train_id] = 0;
        current_velocities[train_id] = 0;
        acceleration_start_time[train_id] = NOT_ACCELERATING;

        // Create notifier
        // TODO(f2fung): If we used only one notifier, our error would be at most a hundreth
        // of a second which is about 5.33mm versus whatever delays we're getting for more notifiers
        notifier_tids[train_id] = Create(MED_PRI, &distance_notifier);
        Send(notifier_tids[train_id], (char *)&msg.train, sizeof(int), (void *)0, 0);
        break;
      case DS_UPDATE_SPEED:
        Reply(tid, (void *)0, 0);
        train_id = tr_num_to_idx(msg.train);

        current_speeds[train_id] = msg.speed;
        acceleration_start_time[train_id] = Time();
        break;
      case DS_GET_UPDATE:
        waiting_tid = tid;
        break;
      case DS_NOTIFIER:
        Reply(tid, (void *)0, 0);
        train_id = tr_num_to_idx(msg.train);

        if (acceleration_start_time[train_id] != NOT_ACCELERATING) {
          if (current_speeds[train_id] > 0) {
            dt = Time() - acceleration_start_time[train_id];
            target_velocity = mean_velocity(msg.train, current_speeds[train_id]);
            current_velocities[train_id] = accelerate(msg.train, current_velocities[train_id], target_velocity, dt);
            if (current_velocities[train_id] == target_velocity) {
              acceleration_start_time[train_id] = NOT_ACCELERATING;
            }
          }
        }

        if (acceleration_start_time[train_id] == NOT_ACCELERATING) {
          dx = mean_velocity(msg.train, current_speeds[train_id]) / 1000;
        } else {
          if (current_speeds[train_id] > 0) {
            dx = current_velocities[train_id] / 1000;
          } else {
            // While stopping
            dt = Time() - acceleration_start_time[train_id];
            if (dt >= DEFAULT_STOPPING_TICKS) {
              current_velocities[train_id] = 0;
              acceleration_start_time[train_id] = NOT_ACCELERATING;
              dx = DEFAULT_STOPPING_DISTANCE * 1000;
            } else {
              dx = 0;
            }
          }
        }

        loc = get_train_location(&train_locations, msg.train);
        if (loc) {
          dx = (dx * piecewise_velocity(msg.train, current_speeds[train_id], loc)) / 100;
        }
        accumulated_dx[train_id] += dx;

        if (waiting_tid == 0) {
          // No-one to notify, so we accumulate dx. This is a bad sign, we generally shouldn't be missing updates.
        } else {
          dum.train = msg.train;
          dum.dx = accumulated_dx[train_id];
          dum.stopping_distance = stopping_distance(msg.train, current_velocities[train_id]);
          /*dum.stopping_time = stopping_time(msg.train, current_velocities[train_id]);*/
          dum.stopping_time = -1; // XXX: DON'T USE THIS PARAM

          Reply(waiting_tid, (char *)&dum, sizeof(distance_update_message));
          waiting_tid = 0;
          accumulated_dx[train_id] = 0;
        }

        break;
      case DS_LOCATION_UPDATE:
        Reply(tid, (void *)0, 0);
        memcpy((char *)&train_locations, (const char *)&msg.loc_array, sizeof(location_array));
        break;
    }
  }

  Exit();
}

/*
 * Public Methods
 */

void start_distance_server() {
  distance_server_tid = Create(MED_PRI, &distance_server);
}

void ds_track_train(int train) {
  distance_server_message msg;
  msg.type = DS_TRACK_TRAIN;
  msg.train = train;
  Send(distance_server_tid, (char *)&msg, sizeof(distance_server_message), (void *)0, 0);
}

void ds_update_speed(int train, int speed) {
  distance_server_message msg;
  msg.type = DS_UPDATE_SPEED;
  msg.train = train;
  msg.speed = speed;
  Send(distance_server_tid, (char *)&msg, sizeof(distance_server_message), (void *)0, 0);
}

void ds_get_update(int* train, int* dx, int* stopping_distance, int* stopping_time) {
  distance_server_message msg;
  distance_update_message dum;
  msg.type = DS_GET_UPDATE;
  Send(distance_server_tid, (char *)&msg, sizeof(distance_server_message), (char *)&dum, sizeof(distance_update_message));

  *train = dum.train;
  *dx = dum.dx;
  *stopping_distance = dum.stopping_distance;
  *stopping_time = dum.stopping_time;
}
