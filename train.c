#include "train.h"
#include "syscall.h"
#include "uart.h"
#include "ourio.h"
#include "debug.h"
#include "priorities.h"
#include "location_server.h"
#include "sensor_server.h"
#include "ourlib.h"
#include "distance_server.h"

/*
 * Note: Some of this global memory might be problematic as the reverse
 * server is also using it...
 * But it should be ok, since we shouldn't be talking to the same train from
 * two different places.
 */
static int train_speeds[NUM_TRAINS + 1];
static int switch_directions[NUM_SWITCHES + 1];
static int reverse_server_tid;
static int tracked_trains[NUM_TRAINS + 1];

/*
 * Helpers for sending commands to the train
 */

void fill_set_speed(char*cmd, int speed, int train) {
  cmd[0] = speed;
  cmd[1] = train;
}

void send_reverse_command(int train) {
  char cmd[2];
  fill_set_speed(cmd, 15, train);
  putbytes(COM1, cmd, 2);
}

void set_speed(int speed, int train) {
  ASSERT(train > 0 && train <= NUM_TRAINS, "train.c: tr_set_speed: Invalid train number");
  ASSERT(speed >= 0 && speed <= NUM_SPEEDS, "train.c: tr_set_speed: Invalid speed");

  char cmd[2];
  fill_set_speed(cmd, speed, train);

  putbytes(COM1, cmd, 2);
  train_speeds[train] = speed;

  if (tracked_trains[train]) {
    ds_update_speed(train, speed);
  }
}

void tr_reverse_task() {
  int tid;
  int train_info[2];
  while (1) {
    Receive(&tid, (char *)train_info, 2*sizeof(int));
    Reply(tid, (void *)0, 0);

    int train = train_info[0];
    int speed = train_info[1];

    set_speed(0, train);

    Delay(300);
    send_reverse_command(train);
    set_speed(speed, train);
  }
  Exit();
}

void reverse(int train) {
  ASSERT(train > 0 && train <= NUM_TRAINS, "train.c: tr_reverse: Invalid train number");

  int train_info[2];
  train_info[0] = train;
  train_info[1] = train_speeds[train];

  Send(reverse_server_tid, (char *)train_info, 2*sizeof(int), (void *)0, 0);
}

int convert_switch_number(int switch_number) {
  if (switch_number < 19) return switch_number;

  switch (switch_number) {
  case 0x99:
    return 19;
  case 0x9A:
    return 20;
  case 0x9B:
    return 21;
  case 0x9C:
    return 22;
  }

  return 0;
}

void sw(int switch_number, char switch_direction) {
  int switch_direction_code;
  switch (switch_direction) {
  case 'S':
    switch_direction_code = 33;
    break;
  case 'C':
    switch_direction_code = 34;
    break;
  default:
    ERROR("train.c: tr_sw: Invalid switch direction");
    return;
  }

  char cmd[3];
  cmd[0] = switch_direction_code;
  cmd[1] = switch_number;
  cmd[2] = 32; /* Turn solenoid off */
  putbytes(COM1, cmd, 3);

  switch_directions[convert_switch_number(switch_number)] = switch_direction;
}

/*
 * Train Controller
 */

#define FINDING_LOCATION_SPEED 3

static int train_controller_tid;

typedef enum train_controller_message_type {
  TRACK_TRAIN,
  SET_ROUTE,
  CHANGE_SPEED,
  REVERSE,
  CHANGE_SWITCH,
  UPDATE_LOCATIONS,
} train_controller_message_type;

typedef struct user_command_data {
  int train;
  int speed;
  int switch_number;
  char switch_direction;
} user_command_data;

typedef struct train_controller_message {
  train_controller_message_type type;
  union {
    user_command_data user_cmd;
    location_array loc_array;
  };
} train_controller_message;

void location_notifier() {
  train_controller_message msg;
  msg.type = UPDATE_LOCATIONS;
  while (1) {
    get_location_updates(&msg.loc_array);
    Send(train_controller_tid, (char *)&msg, sizeof(train_controller_message), (void* )0, 0);
  }
}

void train_controller() {
  location_array train_locations;

  int tid, i;
  train_controller_message msg;
  while (1) {
    Receive(&tid, (char *)&msg, sizeof(train_controller_message));
    switch (msg.type) {
      case TRACK_TRAIN: {
        Reply(tid, (void *)0, 0);
        set_speed(FINDING_LOCATION_SPEED, msg.user_cmd.train);

        // Find location of train.
        sensor_array s_array;
        s_array.num_sensors = get_sensor_data(s_array.sensors, MAX_NEW_SENSORS);
        set_speed(0, msg.user_cmd.train);
        for (i = 0; i < s_array.num_sensors; i++) {
          // TODO(dzelemba): Check for another train stopped on a sensor.
          location loc;
          loc.train = msg.user_cmd.train;
          sensor_copy(&loc.s, &s_array.sensors[i]);

          // TODO(dzelemba): Figure out why we need a direction.
          loc.d = FORWARD;
          track_train(msg.user_cmd.train, &loc);
        }
        tracked_trains[msg.user_cmd.train] = 1;
        break;
      }
      case SET_ROUTE:
        Reply(tid, (void *)0, 0);
        break;
      case CHANGE_SPEED:
        Reply(tid, (void *)0, 0);
        set_speed(msg.user_cmd.speed, msg.user_cmd.train);
        break;
      case REVERSE:
        Reply(tid, (void *)0, 0);
        reverse(msg.user_cmd.train);
        break;
      case CHANGE_SWITCH:
        Reply(tid, (void *)0, 0);
        sw(msg.user_cmd.switch_number, msg.user_cmd.switch_direction);
        break;
      case UPDATE_LOCATIONS:
        Reply(tid, (void *)0, 0);
        memcpy((char *)&train_locations, (const char *)&msg.loc_array, sizeof(location_array));
        // TODO(dzelemba).
        break;
    }
  }

  Exit();
}

/*
 * Public Methods
 */

void init_trains() {
  int i;
  for (i = 1; i < NUM_TRAINS + 1; i++) {
    train_speeds[i] = 0;
    tracked_trains[i] = 0;
  }

  for (i = 1; i < NUM_SWITCHES + 1; i++) {
    switch_directions[i] = '?';
  }

  reverse_server_tid = Create(MED_PRI_K, &tr_reverse_task);
  train_controller_tid = Create(MED_PRI, &train_controller);
  Create(MED_PRI, &location_notifier);
}

/* Train Methods */

void tr_track(int train) {
  train_controller_message msg;
  msg.type = TRACK_TRAIN;
  msg.user_cmd.train = train;
  Send(train_controller_tid, (char *)&msg, sizeof(train_controller_message), (void *)0, 0);
}

void tr_set_speed(int speed, int train) {
  train_controller_message msg;
  msg.type = CHANGE_SPEED;
  msg.user_cmd.train = train;
  msg.user_cmd.speed = speed;
  Send(train_controller_tid, (char *)&msg, sizeof(train_controller_message), (void *)0, 0);
}

void tr_reverse(int train) {
  train_controller_message msg;
  msg.type = REVERSE;
  msg.user_cmd.train = train;
  Send(train_controller_tid, (char *)&msg, sizeof(train_controller_message), (void *)0, 0);
}

/* Switch Methods */

void tr_sw(int switch_number, char switch_direction) {
  train_controller_message msg;
  msg.type = CHANGE_SWITCH;
  msg.user_cmd.switch_direction = switch_direction;
  msg.user_cmd.switch_number = switch_number;
  Send(train_controller_tid, (char *)&msg, sizeof(train_controller_message), (void *)0, 0);
}
