#include "debug.h"
#include "distance_server.h"
#include "ourio.h"
#include "physics.h"
#include "priorities.h"
#include "queue.h"
#include "syscall.h"
#include "task.h"
#include "train.h"

/*
 * Private Methods
 */

static int distance_server_tid;

typedef enum distance_server_message_type {
  DS_TRACK_TRAIN,
  DS_UPDATE_SPEED,
  DS_GET_UPDATE,
  DS_NOTIFIER
} distance_server_message_type;

typedef struct distance_server_message {
  distance_server_message_type type;
  int train;
  int speed;
} distance_server_message;

void distance_notifier() {
  // First we get what train we're a notifier for.
  int train, tid;
  Receive(&tid, (char *)&train, sizeof(int));
  Reply(tid, (void *)0, 0);

  // Now we do our job.
  distance_server_message msg;
  msg.type = DS_NOTIFIER;
  msg.train = train;

  // TODO(f2fung): There's currently only one train to model
  int now = Time();
  while (1) {
    // Use DelayUntil to reduce to constant factor drifting
    DelayUntil(now + 1);
    now++;
    Send(distance_server_tid, (char *)&msg, sizeof(distance_server_message), (void *)0, 0);
  }

  Exit();
}

void distance_server() {
  int waiting_tid = 0;

  int cur_speeds[NUM_TRAINS + 1];
  int notifier_tids[NUM_TRAINS + 1];
  int accumulated_dx[NUM_TRAINS + 1];

  int tid;
  int dx;
  int ret[2];

  distance_server_message msg;
  while (1) {
    Receive(&tid, (char *)&msg, sizeof(distance_server_message));
    switch (msg.type) {
      case DS_TRACK_TRAIN:
        Reply(tid, (void *)0, 0);
        cur_speeds[msg.train] = 0;
        accumulated_dx[msg.train] = 0;

        // Create notifier
        // TODO(f2fung): If we used only one notifier, our error would be at most a hundreth
        // of a second which is about 5.33mm versus whatever delays we're getting for more notifiers
        notifier_tids[msg.train] = Create(MED_PRI, &distance_notifier);
        Send(notifier_tids[msg.train], (char *)&msg.train, sizeof(int), (void *)0, 0);
        break;
      case DS_UPDATE_SPEED:
        Reply(tid, (void *)0, 0);
        // TODO(f2fung): Acceleration/Decceleration/Stopping
        cur_speeds[msg.train] = msg.speed;
        break;
      case DS_GET_UPDATE:
        waiting_tid = tid;
        break;
      case DS_NOTIFIER:
        Reply(tid, (void *)0, 0);

        // TODO(f2fung): Acceleration/Decceleration modelling
        if (cur_speeds[msg.train] == 0) {
          dx = 0;
        } else if (cur_speeds[msg.train] == 11) {
          dx = VMAX_MICROMETERS_PER_TICK;
        }

        accumulated_dx[msg.train] += dx;

        if (waiting_tid == 0) { // This is bad
        } else {
          ret[0] = msg.train;
          ret[1] = accumulated_dx[msg.train];
          Reply(waiting_tid, (char *)ret, sizeof(int) * 2);
          waiting_tid = 0;
          accumulated_dx[msg.train] = 0;
        }

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

void ds_get_update(int* train, int* dx) {
  int ret[2];
  distance_server_message msg;
  msg.type = DS_GET_UPDATE;
  Send(distance_server_tid, (char *)&msg, sizeof(distance_server_message), (char *)ret, sizeof(int) * 2);

  *train = ret[0];
  *dx = ret[1];
}
