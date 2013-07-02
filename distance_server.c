#include "distance_server.h"
#include "syscall.h"
#include "train.h"
#include "queue.h"
#include "task.h"
#include "priorities.h"

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
  int delay = 0; // We get our delay times through replies.
  distance_server_message msg;
  msg.type = DS_NOTIFIER;
  msg.train = train;
  while (1) {
    if (delay != 0) {
      Delay(delay);
    }
    Send(distance_server_tid, (char *)&msg, sizeof(distance_server_message), (char *)&delay, sizeof(int));
  }

  Exit();
}

void ds_reply_to_tasks(queue* waiting_tasks, int* train) {
  while (!is_queue_empty(waiting_tasks)) {
    Reply(pop(waiting_tasks), (char *)train, sizeof(int));
  }
}

int get_delay(int train, int speed) {
  // TODO(dzelemba): Implement this once we have configuration data.
  return 2;
}

void reply_to_notifier(int tid, int train, int speed) {
  int delay = get_delay(train, speed);
  Reply(tid, (char *)&delay, sizeof(int));
}

void distance_server() {
  queue waiting_tasks;
  int q_mem[MAX_TASKS];
  init_queue(&waiting_tasks, q_mem, MAX_TASKS);

  int cur_speeds[NUM_TRAINS + 1];
  int notifier_tids[NUM_TRAINS + 1];

  int tid;
  distance_server_message msg;
  while (1) {
    Receive(&tid, (char *)&msg, sizeof(distance_server_message));
    switch (msg.type) {
      case DS_TRACK_TRAIN:
        Reply(tid, (void *)0, 0);
        cur_speeds[msg.train] = 0;

        // Create notifier
        notifier_tids[msg.train] = Create(MED_PRI, &distance_notifier);
        Send(notifier_tids[msg.train], (char *)&msg.train, sizeof(int), (void *)0, 0);
        break;
      case DS_UPDATE_SPEED:
        Reply(tid, (void *)0, 0);
        // Check if we have to wake up the notifier.
        // NOTE: This assumes that a speed will never go from non zero -> 0 -> non zero
        // faster than a notifier will trigger. Otherwise, we'll reply to a non-send blocked
        // task.
        if (cur_speeds[msg.train] == 0) {
          reply_to_notifier(notifier_tids[msg.train], msg.train, msg.speed);
        }
        cur_speeds[msg.train] = msg.speed;
        break;
      case DS_GET_UPDATE:
        push(&waiting_tasks, tid);
        break;
      case DS_NOTIFIER:
        ds_reply_to_tasks(&waiting_tasks, &msg.train);

        // If speed is 0 let the notifier remain send blocked
        // and we can wake it up again when the speed becomes non-zero.
        if (cur_speeds[msg.train] != 0) {
          reply_to_notifier(notifier_tids[msg.train], msg.train, cur_speeds[msg.train]);
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

void ds_get_update(int* train) {
  distance_server_message msg;
  msg.type = DS_GET_UPDATE;
  Send(distance_server_tid, (char *)&msg, sizeof(distance_server_message), (char *)train, sizeof(int));
}
