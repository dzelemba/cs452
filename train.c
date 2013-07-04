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
#include "track_node.h"
#include "dijkstra.h"
#include "linked_array.h"
#include "track_data.h"

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

int switch_number_from_index(int index) {
  if (index < 19) return index;

  switch (index) {
  case 19:
    return 0x99;
  case 20:
    return 0x9A;
  case 21:
    return 0x9B;
  case 22:
    return 0x9C;
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

#define FINDING_LOCATION_SPEED 2

static int train_controller_tid;

typedef enum train_controller_message_type {
  TRACK_TRAIN,
  SET_ROUTE,
  CHANGE_SPEED,
  TR_REVERSE,
  CHANGE_SWITCH,
  UPDATE_LOCATIONS,
} train_controller_message_type;

typedef struct user_command_data {
  int train;
  int speed;
  int switch_number;
  char switch_direction;
} user_command_data;

typedef struct set_route_command_data {
  int train;
  int speed;
  location dest;
} set_route_command_data;

typedef struct train_controller_message {
  train_controller_message_type type;
  union {
    user_command_data user_cmd;
    location_array loc_array;
    set_route_command_data set_route_data;
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

#define STOPPING_DISTANCE 700

void check_switch_action(sequence* path_node) {
  if (path_node->action == TAKE_STRAIGHT) {
    sw(get_track_node(get_track(), path_node->location)->num, 'S');
  } else if (path_node->action == TAKE_CURVE) {
    sw(get_track_node(get_track(), path_node->location)->num, 'C');
  }
}

track_edge* get_next_edge_in_path(sequence* path_node) {
  track_node* node = get_track_node(get_track(), path_node->location);
  if (node->type == NODE_BRANCH) {
    if (path_node->action == TAKE_STRAIGHT) {
      return &node->edge[DIR_STRAIGHT];
    } else if (path_node->action == TAKE_CURVE) {
      return &node->edge[DIR_CURVED];
    } else {
      ERROR("train.c: get_next_edge_in_path: invalid action for branch node");
    }
  }
  return &node->edge[DIR_AHEAD];
}

// TODO(dzelemba): This assumes our path allows us to reach top speed.
int perform_path_actions(int train, sequence* path_base, int path_size) {
  // We need to turn switches three nodes ahead.
  check_switch_action(path_base);
  if (path_size > 1) check_switch_action(path_base + 1);
  if (path_size > 2) check_switch_action(path_base + 2);

  // Reverse and stop commands need to look out "stopping distance" ahead.
  // TODO(dzelemba): We're stopping too early here.
  int dist = 0, i;
  for (i = 0; i < path_size && dist < STOPPING_DISTANCE; i++) {
    if (path_base[i].action == REVERSE) {
      // Modify path so that we don't reverse twice
      path_base[i].action = DO_NOTHING;

      reverse(train);
      return 0;
    }

    dist += get_next_edge_in_path(&path_base[i])->dist;
  }

  // If there's less than STOPPING_DISTANCE left in the path STOP!
  // TODO(dzelemba): Use direction to improve accuracy.
  if (dist < STOPPING_DISTANCE) {
    set_speed(0, train);
    return 1;
  }

  return 0;
}

int handle_path(int train, sequence* path, int* path_index, int path_size, location* cur_loc) {
  int p_index = *path_index;

  // Perform actions if we've advanced.
  if (path[p_index].location == get_track_index(get_track(), cur_loc)) {
    *path_index = p_index + 1;
    return perform_path_actions(train, path + p_index, path_size - *path_index);
  }

  return 0;
}

void train_controller() {
  location_array train_locations;

  // Paths. We will use kmalloc to allocate paths,
  // so we don't have to pre-allocate a ton of memory.
  int path_sizes[NUM_TRAINS + 1];
  int path_indexes[NUM_TRAINS + 1];
  sequence* paths[NUM_TRAINS+ 1];
  int i = 0;
  for (i = 0; i < NUM_TRAINS + 1; i++) {
    paths[i] = 0;
    path_sizes[i] = 0;
  }

  linked_array trains_on_route;
  la_create(&trains_on_route, NUM_TRAINS);
  linked_array_iterator la_it;

  int tid, train;
  location* cur_loc;
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
          sensor* s = &s_array.sensors[i];
          loc.train = msg.user_cmd.train;
          loc.node = get_track_node(get_track(), sensor2idx(s->group, s->socket));

          // TODO(dzelemba): Figure out why we need a direction.
          loc.d = FORWARD;
          track_train(msg.user_cmd.train, &loc);
        }
        tracked_trains[msg.user_cmd.train] = 1;

        // Allocate memory for future routes.
        paths[msg.user_cmd.train] = (sequence *)kmalloc(TRACK_MAX * sizeof(sequence));
        break;
      }
      case SET_ROUTE:
        train = msg.set_route_data.train;
        ASSERT(paths[train] != 0, "train.c: set_route on non-tracked train");
        Reply(tid, (void *)0, 0);

        cur_loc = get_train_location(&train_locations, train);
        la_insert(&trains_on_route, train, (void*)train);

        path_indexes[train] = 0;
        get_path(get_track(), cur_loc, &msg.set_route_data.dest, paths[train], &path_sizes[train]);
        set_speed(msg.set_route_data.speed, train);
        handle_path(train, paths[train], &path_indexes[train], path_sizes[train], cur_loc);
        break;
      case CHANGE_SPEED:
        Reply(tid, (void *)0, 0);
        set_speed(msg.user_cmd.speed, msg.user_cmd.train);
        break;
      case TR_REVERSE:
        Reply(tid, (void *)0, 0);
        reverse(msg.user_cmd.train);
        break;
      case CHANGE_SWITCH:
        Reply(tid, (void *)0, 0);
        sw(msg.user_cmd.switch_number, msg.user_cmd.switch_direction);
        break;
      case UPDATE_LOCATIONS: {
        Reply(tid, (void *)0, 0);
        memcpy((char *)&train_locations, (const char *)&msg.loc_array, sizeof(location_array));

        la_it_create(&trains_on_route, &la_it);
        while (la_it_has_next(&trains_on_route, &la_it)) {
          train = (int)la_it_get_next(&trains_on_route, &la_it);
          if (handle_path(train, paths[train], &path_indexes[train],
                          path_sizes[train], get_train_location(&train_locations, train))) {
            la_remove(&trains_on_route, train);
          }
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

void init_trains() {
  int i;
  for (i = 1; i < NUM_TRAINS + 1; i++) {
    train_speeds[i] = 0;
    tracked_trains[i] = 0;
  }

  for (i = 1; i < NUM_SWITCHES + 1; i++) {
    sw(switch_number_from_index(i), 'S');
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
  msg.type = TR_REVERSE;
  msg.user_cmd.train = train;
  Send(train_controller_tid, (char *)&msg, sizeof(train_controller_message), (void *)0, 0);
}

void tr_set_route(int train, int speed, location* loc) {
  train_controller_message msg;
  msg.type = SET_ROUTE;
  msg.set_route_data.train = train;
  msg.set_route_data.speed = speed;
  memcpy((char *)&msg.set_route_data.dest, (const char *)loc, sizeof(location));
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

char get_switch_direction(int switch_number) {
  return switch_directions[convert_switch_number(switch_number)];
}
