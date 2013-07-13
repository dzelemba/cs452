#include "debug.h"
#include "dijkstra.h"
#include "distance_server.h"
#include "linked_array.h"
#include "location.h"
#include "location_server.h"
#include "ourio.h"
#include "ourlib.h"
#include "physics.h"
#include "priorities.h"
#include "sensor_server.h"
#include "syscall.h"
#include "track_data.h"
#include "track_node.h"
#include "train.h"
#include "uart.h"
#include "physics.h"
#include "user_prompt.h"

/*
 * Note: Some of this global memory might be problematic as the reverse
 * server is also using it...
 * But it should be ok, since we shouldn't be talking to the same train from
 * two different places.
 */
static int _train_num_to_idx[NUM_TRAINS];
static int _train_idx_to_num[MAX_TRAINS];

static int train_speeds[MAX_TRAINS];
static int tracked_trains[MAX_TRAINS];
static int switch_directions[NUM_SWITCHES + 1];

static int reverse_server_tid;

void assign_train_to_idx(int train, int idx) {
  _train_num_to_idx[train] = idx;
  _train_idx_to_num[idx] = train;
}

int tr_num_to_idx(int train) {
  return _train_num_to_idx[train];
}

int tr_idx_to_num(int idx) {
  return _train_idx_to_num[idx];
}

/*
 * Helpers for sending commands to the train
 */

void fill_set_speed(char*cmd, int speed, int train) {
  cmd[0] = speed;
  cmd[1] = train;
}

void send_reverse_command(int train) {
  int train_idx = tr_num_to_idx(train);

  char cmd[2];
  fill_set_speed(cmd, 15, train);
  putbytes(COM1, cmd, 2);
  if (tracked_trains[train_idx]) {
    ls_train_reversed(train);
  }
}

void set_speed(int speed, int train) {
  ASSERT(train > 0 && train <= NUM_TRAINS, "train.c: tr_set_speed: Invalid train number");
  ASSERT(speed >= 0 && speed <= NUM_SPEEDS, "train.c: tr_set_speed: Invalid speed");

  int train_idx = tr_num_to_idx(train);

  char cmd[2];
  fill_set_speed(cmd, speed, train);
  train_speeds[train_idx] = speed;

  putbytes(COM1, cmd, 2);
  if (tracked_trains[train_idx]) {
    ds_update_speed(train, speed);
  }
}

typedef struct reverse_command {
  int train;
  int speed;
  int delay;
} reverse_command;

void tr_reverse_task() {
  int tid;
  reverse_command msg;
  while (1) {
    Receive(&tid, (char *)&msg, sizeof(reverse_command));
    Reply(tid, (void *)0, 0);

    set_speed(0, msg.train);

    Delay(msg.delay);
    send_reverse_command(msg.train);
    set_speed(msg.speed, msg.train);
  }
  Exit();
}

void reverse(int train, int delay) {
  ASSERT(train > 0 && train <= NUM_TRAINS, "train.c: tr_reverse: Invalid train number");

  reverse_command msg;
  msg.train = train;
  msg.speed = train_speeds[tr_num_to_idx(train)];
  msg.delay = delay;

  Send(reverse_server_tid, (char *)&msg, sizeof(reverse_command), (void *)0, 0);
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

  Exit();
}

void perform_switch_action(sequence* path_node) {
  if (!path_node->performed_action) {
    if (path_node->action == TAKE_STRAIGHT) {
      sw(get_track_node(get_track(), path_node->location)->num, 'S');
    } else if (path_node->action == TAKE_CURVE) {
      sw(get_track_node(get_track(), path_node->location)->num, 'C');
    } else {
      ERROR("train.c: perform_switch_action: node doesn't have switch action");
    }
    path_node->performed_action = 1;
  }
}

void perform_reverse_action(sequence* path_node, int train, int stopping_time) {
  if (!path_node->performed_action) {
    reverse(train, stopping_time);
    path_node->performed_action = 1;
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
      // Case when branch node is our last node.
      return &node->edge[DIR_AHEAD];
    }
  } else if (node->type == NODE_EXIT) {
    return 0;
  }

  return path_node->action == REVERSE ? 0 : &node->edge[DIR_AHEAD];
}

// Upper bound for how far ahead we need to lookahead to switch
// a switch in mm. Determined by upper limit for train speed of
// 50 cm/s and byte tranmission rate of 1 byte/ 4ms. This gives
// 4mm lookahead distance. Considering each train might send
// another command with the switch we get 2 * 4mm * MAX_TRAINS.
// Round up 8mm to 1cm.
#define SWITCH_LOOKAHEAD_DISTANCE (10 * MAX_TRAINS)

// Maximum error in our distance measurements in mm.
#define MAX_DISTANCE_ERROR 150

/*
 * Note: When talking about wheels of the train we will use
 *   - Backward/Forward when referring to the direction the
 *     train is moving.
 *   - Back/Front when referring to the side of the train the
 *     pickup is on.
 */

#define PICKUP_TO_FRONTWHEEL 0
#define PICKUP_LENGTH 50
#define PICKUP_TO_BACKWHEEL 120

int get_distance_to_forwardwheel(direction d) {
  switch (d) {
    case FORWARD:
      return PICKUP_TO_FRONTWHEEL;
    case BACKWARD:
      return PICKUP_TO_BACKWHEEL;
  }
  ERROR("train.c: Invalid distance given: %d\n", d);
  return 0;
}

int get_distance_to_backwardwheel(direction d) {
  switch (d) {
    case FORWARD:
      return PICKUP_TO_BACKWHEEL + PICKUP_LENGTH;
    case BACKWARD:
      return PICKUP_TO_FRONTWHEEL + PICKUP_LENGTH;
  }
  ERROR("train.c: Invalid distance given: %d\n", d);
  return 0;
}

// TODO(dzelemba): Fix this to incorporate SWITCH_OFFSET.
// once we have reverses at branch nodes.
int get_reverse_lookahead(int stopping_distance, direction d) {
  return max(stopping_distance - get_distance_to_backwardwheel(d) - MAX_DISTANCE_ERROR, 0);
}

int get_switch_lookahead(direction d) {
  return SWITCH_LOOKAHEAD_DISTANCE + get_distance_to_forwardwheel(d) + MAX_DISTANCE_ERROR;
}

int get_stop_lookahead(int stopping_distance, direction d) {
  return stopping_distance + get_distance_to_forwardwheel(d);
}

void perform_all_path_actions(int train, sequence* path_base, int path_size) {
  int i;
  for (i = 0; i < path_size; i++) {
    sequence_action action = path_base[i].action;
    if (action == TAKE_STRAIGHT || action == TAKE_CURVE) {
      perform_switch_action(&path_base[i]);
    }
  }
}

int perform_path_actions(int train, sequence* path_base, int path_size, location* cur_loc) {
  // This really shouldn't happen. If it does, it must because
  // we're trying to reverse at an end edge and have gone too
  // far, so just reverse.
  if (cur_loc->cur_edge == 0) {
    if (path_base->action == REVERSE) {
      perform_reverse_action(path_base, train, MAX_STOPPING_TIME);
    }
  }

  int reverse_lookahead = get_reverse_lookahead(cur_loc->stopping_distance, cur_loc->d);
  int switch_lookahead = get_switch_lookahead(cur_loc->d);
  int stop_lookahead = get_stop_lookahead(cur_loc->stopping_distance, cur_loc->d);
  int max_lookahead = max(reverse_lookahead, max(switch_lookahead, stop_lookahead));

  track_edge* next_edge = 0;
  int dist = max(cur_loc->cur_edge->dist - cur_loc->um_past_node / 1000, 0), i;
  for (i = 0; i < path_size && dist <= max_lookahead; i++) {
    sequence_action action = path_base[i].action;
    if (action == REVERSE && dist <= reverse_lookahead) {
      perform_reverse_action(&path_base[i], train, MAX_STOPPING_TIME);
    } else if ((action == TAKE_STRAIGHT || action == TAKE_CURVE) && dist <= switch_lookahead) {
      perform_switch_action(&path_base[i]);
    }

    next_edge = get_next_edge_in_path(&path_base[i]);
    if (next_edge == 0) {
      break; // Hit reverse node.
    }

    // Last edge isn't part of the path.
    if (i != path_size - 1) {
      dist += next_edge->dist;
    }
  }

  // Check if we've hit the end of the path (not just a reverse node)
  // and the reminaing distance is less than stop_lookahead.
  if (i == path_size && dist <= stop_lookahead) {
    set_speed(0, train);
    // Turn any remaining switches.
    perform_all_path_actions(train, path_base, path_size);
    return 1;
  }

  return 0;
}

int perform_initial_path_actions(int train, sequence* path, int path_size, location* cur_loc) {
  sequence_action action = path->action;
  if (action == REVERSE) {
    perform_reverse_action(path, train, MAX_STOPPING_TIME);
  } else if (action == TAKE_STRAIGHT || action == TAKE_CURVE) {
    ERROR("train.c: First node in path is a branch, should never happen");
  }

  // TODO(dzelemba): Check for trivial paths.
  return 0;
}

int handle_path(int train, sequence* path, int* path_index, int path_size, location* cur_loc) {
  int p_index = *path_index;
  int path_nodes_left = path_size - p_index - 1;
  int cur_loc_index = get_track_index(get_track(), cur_loc);

  /*
   * The first case here looks for when we have just finished reversing
   * and are starting to move again in the opposite direction.
   *
   * The other case is the regular case, where we check if we've advanced along
   * the path, or if we've missed an update along the path.
   */
  if (path[p_index].action == REVERSE && cur_loc->cur_edge != 0 &&
      node2idx(get_track(), cur_loc->cur_edge->dest) == path[p_index + 1].location) {
    INFO(TRAIN_CONTROLLER, "Train %d Reversed at %s", train, cur_loc->node->name);
    *path_index = p_index + 1;
  } else {
    if (path[p_index].location == cur_loc_index) {
      INFO(TRAIN_CONTROLLER, "Train %d Advanced to %s", train, cur_loc->node->name);
      *path_index = p_index + 1;
    } else if (path_nodes_left > 1 && path[p_index + 1].location == cur_loc_index) {
      INFO(TRAIN_CONTROLLER, "Train %d Skipped to %s", train, cur_loc->node->name);
      // TODO(dzelemba): Look ahead to next_sensor here instead of next node.
      *path_index = p_index + 2;
    }

    // Don't permit advancing at reverse nodes. We must wait until reversing
    // is complete before advancing the path.
    if (p_index > 0 && path[p_index - 1].action == REVERSE) {
      *path_index -= 1;
    }
  }

  return perform_path_actions(train, path + *path_index, path_size - *path_index, cur_loc);
}

void train_controller() {
  RegisterAs("Train Controller");
  location_array train_locations;
  train_locations.size = 0;

  // Paths. We will use kmalloc to allocate paths,
  // so we don't have to pre-allocate a ton of memory.
  int path_sizes[MAX_TRAINS];
  int path_indexes[MAX_TRAINS];
  sequence* paths[MAX_TRAINS];

  int i = 0;
  for (i = 0; i < MAX_TRAINS; i++) {
    paths[i] = 0;
    path_sizes[i] = 0;
  }

  linked_array trains_on_route;
  la_create(&trains_on_route, MAX_TRAINS);
  linked_array_iterator la_it;

  int tid, train, train_idx;
  location* cur_loc;
  train_controller_message msg;
  while (1) {
    Receive(&tid, (char *)&msg, sizeof(train_controller_message));
    switch (msg.type) {
      case TRACK_TRAIN: {
        Reply(tid, (void *)0, 0);
        train_idx = tr_num_to_idx(msg.user_cmd.train);
        set_speed(FINDING_LOCATION_SPEED, msg.user_cmd.train);

        // Find location of train.
        sensor_array s_array;
        s_array.num_sensors = get_sensor_data(s_array.sensors, MAX_NEW_SENSORS);
        set_speed(0, msg.user_cmd.train);
        for (i = 0; i < s_array.num_sensors; i++) {
          // TODO(dzelemba): Check for another train stopped on a sensor.
          location loc;
          init_location(&loc);

          sensor* s = &s_array.sensors[i];
          loc.train = msg.user_cmd.train;
          loc.node = get_track_node(get_track(), sensor2idx(s->group, s->socket));
          loc.stopping_distance = 0;
          loc.stopping_time = 0;

          // TODO(dzelemba): Figure out why we need a direction.
          loc.d = FORWARD;
          track_train(msg.user_cmd.train, &loc);
        }
        tracked_trains[train_idx] = 1;

        // Allocate memory for future routes.
        paths[train_idx] = (sequence *)kmalloc(TRACK_MAX * sizeof(sequence));
        break;
      }
      case SET_ROUTE:
        train = msg.set_route_data.train;
        train_idx = tr_num_to_idx(train);
        ASSERT(paths[train_idx] != 0, "train.c: set_route on non-tracked train");
        Reply(tid, (void *)0, 0);

        cur_loc = get_train_location(&train_locations, train);
        la_insert(&trains_on_route, train_idx, (void*)train);

        path_indexes[train_idx] = 0;

        // If first node is a branch, then get directions from the next node
        // so we don't have to worry about backing up to clear the switch.
        if (cur_loc->node->type == NODE_BRANCH) {
          get_path(get_track(), get_next_edge(cur_loc->node)->dest, msg.set_route_data.dest.node,
                   paths[train_idx], &path_sizes[train_idx]);
        } else {
          get_path(get_track(), cur_loc->node, msg.set_route_data.dest.node,
                   paths[train_idx], &path_sizes[train_idx]);
        }
        set_speed(msg.set_route_data.speed, train);
        perform_initial_path_actions(train, paths[train_idx], path_sizes[train_idx], cur_loc);
        handle_path(train, paths[train_idx], &path_indexes[train_idx],
                    path_sizes[train_idx], cur_loc);
        break;
      case CHANGE_SPEED:
        Reply(tid, (void *)0, 0);
        set_speed(msg.user_cmd.speed, msg.user_cmd.train);
        break;
      case TR_REVERSE:
        Reply(tid, (void *)0, 0);
        reverse(msg.user_cmd.train, MAX_STOPPING_TIME);
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
          train_idx = tr_num_to_idx(train);
          if (handle_path(train, paths[train_idx], &path_indexes[train_idx],
                          path_sizes[train_idx], get_train_location(&train_locations, train))) {
            la_remove(&trains_on_route, train_idx);
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
  assign_train_to_idx(35, 0);
  assign_train_to_idx(43, 1);
  assign_train_to_idx(45, 2);
  assign_train_to_idx(47, 3);
  assign_train_to_idx(48, 4);
  assign_train_to_idx(49, 5);
  assign_train_to_idx(50, 6);
  assign_train_to_idx(51, 7);

  int i;
  for (i = 0; i < MAX_TRAINS; i++) {
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
