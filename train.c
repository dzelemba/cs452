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
#include "reservation_server.h"
#include "track_edge_array.h"
#include "switch_server.h"

// Uncomment this to enable deadlock detection and resolution.
// #define ENABLE_DEADLOCK_DETECTION 1

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

static int reversing_tasks[MAX_TRAINS];

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

typedef struct reverse_msg {
  int speed;
  int delay;
} reverse_msg;

void tr_reverse_task() {
  int train, tid;
  Receive(&tid, (char *)&train, sizeof(int));
  Reply(tid, NULL, 0);

  reverse_msg msg;
  while (true) {
    Receive(&tid, (char *)&msg, sizeof(reverse_msg));
    Reply(tid, NULL, 0);

    Delay(msg.delay);
    send_reverse_command(train);
    set_speed(msg.speed, train);
  }

  Exit();
}

void reverse(int train, int delay) {
  ASSERT(train > 0 && train <= NUM_TRAINS, "train.c: tr_reverse: Invalid train number");

  reverse_msg msg;
  msg.speed = train_speeds[tr_num_to_idx(train)];
  msg.delay = delay;

  set_speed(0, train);

  int train_idx = tr_num_to_idx(train);
  if (reversing_tasks[train_idx] == 0) {
    reversing_tasks[train_idx] = Create(MED_PRI, &tr_reverse_task);
    Send(reversing_tasks[train_idx], (char *)&train, sizeof(int), NULL, 0);
  }
  Send(reversing_tasks[train_idx], (char *)&msg, sizeof(reverse_msg), NULL, 0);
}

/*
 * Train Controller
 */

#define FINDING_LOCATION_SPEED 1

static int train_controller_tid;

typedef enum train_controller_message_type {
  TRACK_TRAIN,
  SET_ROUTE,
  CHANGE_SPEED,
  TR_REVERSE,
  UPDATE_LOCATIONS,
  RETRY_RESERVATIONS,
  TC_GET_DONE_TRAINS,
  TC_NOTIFY_WHEN_ALL_TRAINS_STOPPED,
  TC_DISABLE_EDGE,
} train_controller_message_type;

typedef struct user_command_data {
  int train;
  int speed;
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
    train_array tr_array;
    track_edge* edge;
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

void rs_notifier() {
  train_controller_message msg;
  msg.type = RETRY_RESERVATIONS;
  while (1) {
    rs_get_updates(&msg.tr_array);
    Send(train_controller_tid, (char *)&msg, sizeof(train_controller_message), (void* )0, 0);
  }

  Exit();
}

/*
 * Path Following
 */

typedef enum path_following_state {
  NOT_STARTED,
  ON_ROUTE,
  DONE
} path_following_state;

typedef struct path_following_info {
  sequence path[TRACK_MAX];
  int path_size;
  int path_index;
  track_edge_array reserved_edges;
  track_edge* blocked_edge;
  int is_stopping;
  int saved_speed;
  path_following_state state;
  track_node* dest;

  // Used when the train receives a restart message, but hasn't
  // stopped moving yet.
  bool restart_when_stopped;
} path_following_info;

/*
 * Here we switch merge switches to be in the direction we're coming from.
 * This is so that if we reverse on top of a merge, we don't derail.
 * It also helps the location_server get the proper direction on reverses.
 */
void perform_merge_action(track_edge* edge, sequence* path_node) {
  if (!path_node->performed_action) {
    track_node* br_node = edge->dest->reverse;
    if (&br_node->edge[DIR_STRAIGHT] == edge->reverse) {
      tr_sw(br_node->num, 'S');
    } else if (&br_node->edge[DIR_CURVED] == edge->reverse) {
      tr_sw(br_node->num, 'C');
    } else {
      ERROR("train.c: perform_merge_action given invalid (edge, node) pair");
    }
    path_node->performed_action = 1;
  }
}

void perform_switch_action(sequence* path_node) {
  if (!path_node->performed_action) {
    if (path_node->action == TAKE_STRAIGHT) {
      tr_sw(get_track_node(path_node->location)->num, 'S');
    } else if (path_node->action == TAKE_CURVE) {
      tr_sw(get_track_node(path_node->location)->num, 'C');
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
  track_node* node = get_track_node(path_node->location);
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

  return &node->edge[DIR_AHEAD];
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

// Use a different value for the reverse error as
// overshooting can be just as bad as undershooting.
#define REVERSE_BUFFER 50

// Distance in cm to keep reserved behind the train.
#define RESERVATION_BUFFER 50

int get_distance_to_forwardwheel(direction d) {
  switch (d) {
    case FORWARD:
      return PICKUP_TO_FRONTWHEEL;
    case BACKWARD:
      return PICKUP_TO_BACKWHEEL;
  }
  ERROR("train.c: Invalid direction given: %d\n", d);
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
  return max(stopping_distance - get_distance_to_backwardwheel(d) - REVERSE_BUFFER, 0);
}

int get_switch_lookahead(direction d) {
  return SWITCH_LOOKAHEAD_DISTANCE + get_distance_to_forwardwheel(d) + MAX_DISTANCE_ERROR;
}

int get_stop_lookahead(int stopping_distance, direction d) {
  return stopping_distance + get_distance_to_forwardwheel(d);
}

int get_reserve_lookahead(int stopping_distance, direction d) {
  return stopping_distance + get_distance_to_forwardwheel(d) + MAX_DISTANCE_ERROR;
}

int get_reservation_lookbehind(direction d) {
  return RESERVATION_BUFFER + get_distance_to_backwardwheel(d);
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

void restart_train(location* loc, path_following_info* p_info) {
  if (loc->stopping_distance == 0) {
    set_speed(p_info->saved_speed, loc->train);
    p_info->is_stopping = 0;
    p_info->blocked_edge = 0;
  } else {
    p_info->restart_when_stopped = true;
  }
}

void stop_train(int train, path_following_info* p_info) {
  int train_idx = tr_num_to_idx(train);
  p_info->saved_speed = train_speeds[train_idx];
  set_speed(0, train);
  p_info->is_stopping = 1;
}

void tc_reserve_edge(int train, track_edge* edge, location* cur_loc,
                  path_following_info* p_info) {
  track_edge_array* reserved_edges = &p_info->reserved_edges;
  if (p_info->blocked_edge != edge && is_edge_free(edge, reserved_edges)) {
    rs_reply reply = rs_reserve(train, edge);
    if (reply == FAIL) {
      stop_train(train, p_info);
      p_info->blocked_edge = edge;
    } else {
      reserve_edge(edge, reserved_edges);
    }
  }
}

void tc_free_edge(int train, track_edge* edge, path_following_info* p_info) {
  track_edge_array* reserved_edges = &p_info->reserved_edges;

  if (!is_edge_free(edge, reserved_edges)) {
    rs_free(train, edge);
    free_edge(edge, reserved_edges);
  }
}

void tc_free_all_edges(int train, path_following_info* p_info) {
  track_node* track = get_track();
  int i, j;

  track_node* node;
  for (i = 0; i < TRACK_MAX; i++) {
    node = get_track_node(i);
    for (j = 0; j < get_num_neighbours(node->type); j++) {
      tc_free_edge(train, &track[i].edge[j], p_info);
    }
  }
}

#define MAX_LOOKBEHIND 1000

void previous_edges_within_distance(track_edge* curr, int cd, int fd, track_edge_array* seen) {
  int i;
  track_node* dest = curr->dest;

  if (cd >= fd) {
    set_edge(seen, curr);
    if (cd >= fd + MAX_LOOKBEHIND) {
      return;
    }
  }

  cd += curr->dist;
  for (i = 0; i < get_num_neighbours(dest->type); i++) {
    previous_edges_within_distance(&dest->edge[i], cd, fd, seen);
  }
}

void tc_free_previous_edges(int train, location* cur_loc, path_following_info* p_info,
                            int distance_behind) {
  track_edge_array freeable_edges;
  clear_track_edge_array(&freeable_edges);

  track_node* rev = cur_loc->node->reverse;

  int i, j;

  int cd = cur_loc->um_past_node / UM_PER_MM;

  // The only time when um_past_node < 0 and we're on a sensor is when
  // we're stopping at which point um_past_node isn't reliable anymore.
  // TODO(dzelemba): Move this idea into the location server.
  if (cd < 0 && cur_loc->node->type == NODE_SENSOR) {
    cd = 0;
  }

  for (i = 0; i < get_num_neighbours(rev->type); i++) {
    previous_edges_within_distance(&rev->edge[i], cd, distance_behind, &freeable_edges);
  }

  track_node* node;
  for (i = 0; i < TRACK_MAX; i++) {
    node = get_track_node(i);
    for (j = 0; j < get_num_neighbours(node->type); j++) {
      if (isset_edge(&freeable_edges, &node->edge[j])) {
        tc_free_edge(train, &node->edge[j], p_info);
      }
    }
  }
}

void perform_path_actions(location* cur_loc, path_following_info* p_info) {
  sequence* path_base = p_info->path + p_info->path_index;
  int train = cur_loc->train;
  int path_size = p_info->path_size - p_info->path_index;

  // This really shouldn't happen. If it does, it must because
  // we're trying to reverse at an end edge and have gone too
  // far, so just reverse.
  if (cur_loc->cur_edge == 0) {
    if (path_base->action == REVERSE) {
      // TODO(f2fung): Reverse delay should rely on the physics of the train
      if (cur_loc->stopping_distance > 0) {
        perform_reverse_action(path_base, train, MAX_STOPPING_TICKS);
      } else {
        perform_reverse_action(path_base, train, 0);
      }
      return;
    }
  }

  int reverse_lookahead = get_reverse_lookahead(cur_loc->stopping_distance, cur_loc->d);
  int switch_lookahead = get_switch_lookahead(cur_loc->d);
  int stop_lookahead = get_stop_lookahead(cur_loc->stopping_distance, cur_loc->d);
  int reserve_lookahead = get_reserve_lookahead(cur_loc->stopping_distance, cur_loc->d);
  int max_lookahead = max(reverse_lookahead, max(switch_lookahead,
                      max(stop_lookahead, reserve_lookahead)));

  track_edge* next_edge = cur_loc->cur_edge;
  int dist = max(cur_loc->cur_edge->dist - cur_loc->um_past_node / UM_PER_MM, 0), i;
  for (i = 0; i < path_size && dist <= max_lookahead; i++) {
    sequence_action action = path_base[i].action;
    if (!p_info->is_stopping && action == REVERSE) {
      if (dist <= reverse_lookahead) {
        p_info->is_stopping = 1;
        INFO(TRAIN_CONTROLLER, "Train %d sent reverse command at %d past %s", train, cur_loc->um_past_node / UM_PER_MM, cur_loc->node->name);
        if (cur_loc->stopping_distance > 0) {
          perform_reverse_action(&path_base[i], train, MAX_STOPPING_TICKS);
        } else {
          perform_reverse_action(&path_base[i], train, 0);
        }

        // Acquire reverse edge.
        next_edge = get_next_edge_in_path(&path_base[i]);
        tc_reserve_edge(train, next_edge, cur_loc, p_info);
      }
      // Break so that we don't start performing actions that need to be performed
      // after the reverse is finished.
      break;
    } else if ((action == TAKE_STRAIGHT || action == TAKE_CURVE) && dist <= switch_lookahead) {
      perform_switch_action(&path_base[i]);
    } else if (next_edge->dest->type == NODE_MERGE && dist <= switch_lookahead) {
      perform_merge_action(next_edge, &path_base[i]);
    }

    next_edge = get_next_edge_in_path(&path_base[i]);
    if (next_edge == 0) {
      break; // Exit node hit.
    }

    if (!p_info->is_stopping && dist <= reserve_lookahead) {
      tc_reserve_edge(train, next_edge, cur_loc, p_info);
    }

    // Last edge isn't part of the path.
    if (i != path_size - 1) {
      dist += next_edge->dist;
    }
  }

  // Check if we've hit the end of the path (not just a reverse node)
  // and the reminaing distance is less than stop_lookahead.
  if (!p_info->is_stopping && i == path_size && dist <= stop_lookahead) {
    stop_train(train, p_info);
    // Turn any remaining switches.
    perform_all_path_actions(train, path_base, path_size);
  }
}

int perform_initial_path_actions(location* cur_loc, path_following_info* p_info) {
  int train = cur_loc->train;
  sequence* path = p_info->path;
  sequence_action action = path->action;
  if (action == REVERSE) {
    p_info->is_stopping = 1;
    if (cur_loc->stopping_distance > 0) {
      perform_reverse_action(path, train, MAX_STOPPING_TICKS);
    } else {
      perform_reverse_action(path, train, 0);
    }
  } else if (action == TAKE_STRAIGHT || action == TAKE_CURVE) {
    perform_switch_action(path);
  }

  // TODO(dzelemba): Check for trivial paths.
  return 0;
}

bool any_reverses_left(path_following_info* p_info) {
  int i;
  for (i = p_info->path_index; i < p_info->path_size; i++) {
    if (p_info->path[i].action == REVERSE) {
      return true;
    }
  }
  return false;
}

void handle_path(location* cur_loc, path_following_info* p_info) {
  int train = cur_loc->train;
  int p_index = p_info->path_index;
  int path_nodes_left = p_info->path_size - p_index - 1;
  int cur_loc_index = get_track_index(get_track(), cur_loc);
  sequence* path = p_info->path;
  int* path_index = &p_info->path_index;

  /*
   * The first case here looks for when we have just finished reversing
   * and are starting to move again in the opposite direction.
   *
   * The other case is the regular case, where we check if we've advanced along
   * the path, or if we've missed an update along the path.
   */
  if (path[p_index].action == REVERSE && cur_loc->cur_edge != 0 &&
      (node2idx(get_track(), cur_loc->cur_edge->dest) == path[p_index + 1].location ||
       node2idx(get_track(), cur_loc->node) == path[p_index + 1].location)) {
    INFO(TRAIN_CONTROLLER, "Train %d Reversed at %s", train, cur_loc->node->name);
    *path_index = p_index + 1;
    p_info->is_stopping = 0;
  } else {
    if (path[p_index].location == cur_loc_index) {
      *path_index = p_index + 1;
    } else if (path_nodes_left > 1 && path[p_index + 1].location == cur_loc_index) {
      INFO(TRAIN_CONTROLLER, "Train %d Skipped to %s", train, cur_loc->node->name);
      // TODO(dzelemba): Look ahead to next_sensor here instead of next node.
      *path_index = p_index + 2;
      tc_free_previous_edges(train, cur_loc, p_info, get_reservation_lookbehind(cur_loc->d));
    }

    // Don't permit advancing at reverse nodes. We must wait until reversing
    // is complete before advancing the path.
    // Second case frees previous edges we no longer need to reserve. Note that
    // we don't free an edge if we're waiting on a reverse command.
    if (*path_index > p_index) {
      if (*path_index > 0 && path[*path_index - 1].action == REVERSE) {
        *path_index -= 1;
      } else if (p_info->state != DONE) {
        INFO(TRAIN_CONTROLLER, "Train %d Advanced to %s", train, cur_loc->node->name);
        tc_free_previous_edges(train, cur_loc, p_info, get_reservation_lookbehind(cur_loc->d));
      }
    }
  }

  if (*path_index >= p_info->path_size) {
    *path_index = p_info->path_size - 1;
  }

  if (p_info->blocked_edge == NULL && p_info->is_stopping &&
      cur_loc->stopping_distance == 0 && !any_reverses_left(p_info)) {
    p_info->state = DONE;
    tc_free_previous_edges(train, cur_loc, p_info, get_reservation_lookbehind(cur_loc->d));
  }

  perform_path_actions(cur_loc, p_info);
}

void check_all_trains_stopped(int* waiting_tid, location_array* train_locations,
                              path_following_info* path_info) {
  if (*waiting_tid == 0) {
    return;
  }

  int i;
  for (i = 0; i < train_locations->size; i++) {
    location *cur_loc = &train_locations->locations[i];
    int train = cur_loc->train;
    int train_idx = tr_num_to_idx(train);
    if ((path_info[train_idx].state == ON_ROUTE && path_info[train_idx].blocked_edge == NULL) ||
        (cur_loc->stopping_distance != 0)) {
      return;
    }
  }

  Reply(*waiting_tid, (char *)train_locations, sizeof(location_array));
  *waiting_tid = 0;
}

void check_done_trains(int* waiting_tid, location_array* train_locations,
                       path_following_info* path_info) {
  train_array done_trains;
  done_trains.size = 0;

  int i;
  for (i = 0; i < train_locations->size; i++) {
    location *cur_loc = &train_locations->locations[i];
    int train = cur_loc->train;
    int train_idx = tr_num_to_idx(train);
    if (path_info[train_idx].state == DONE || path_info[train_idx].state == NOT_STARTED) {
      done_trains.trains[done_trains.size++] = train;
    }
  }
  if (done_trains.size > 0 && *waiting_tid != 0) {
    Reply(*waiting_tid, (char *)&done_trains, sizeof(train_array));
    *waiting_tid = 0;
  }
}

void start_route(location* loc, path_following_info* p_info, track_edge_array* blocked_edges) {
  p_info->path_index = 0;
  p_info->state = ON_ROUTE;

  // If first node is a branch, or if a branch is close enough then get
  // directions from the next node so we don't have to worry about backing
  // up to clear the switch.
  if (loc->node->type == NODE_BRANCH) {
    set_edge(blocked_edges, get_other_branch_edge(loc->node, loc->cur_edge));
  }
  if (loc->cur_edge != NULL && loc->cur_edge->dest->type == NODE_BRANCH &&
      loc->cur_edge->dist - loc->um_past_node / UM_PER_MM <= MAX_DISTANCE_ERROR) {
    set_edge(blocked_edges, get_other_branch_edge(loc->cur_edge->dest,
             get_next_edge(loc->cur_edge->dest)));
  }
  if (loc->node->type == NODE_MERGE) {
    set_edge(blocked_edges, get_other_branch_edge(loc->node->reverse,
             get_next_edge(loc->node->reverse)));
  }

  if (loc->cur_edge != NULL) {
    track_edge* prev_edge = get_next_edge(loc->node->reverse);
    if (prev_edge != NULL && prev_edge->dest->type == NODE_BRANCH &&
        (loc->um_past_node / UM_PER_MM - get_distance_to_backwardwheel(loc->d) +
           prev_edge->dist < MAX_DISTANCE_ERROR)) {
      set_edge(blocked_edges, get_other_branch_edge(prev_edge->dest, get_next_edge(prev_edge->dest)));
    }
  }

  get_path(get_track(), loc->node, p_info->dest, blocked_edges, p_info->path, &p_info->path_size);

  // Check if the edge we're blocked in is in the other direction.
  if (p_info->blocked_edge != NULL) {
    int i;
    bool blocked_edge_in_path = false;
    // Only look 3 steps ahead for now.
    for (i = 0; i < 3; i++) {
      track_edge* next_edge = get_next_edge_in_path(&p_info->path[i]);
      if (next_edge == p_info->blocked_edge || next_edge == p_info->blocked_edge->reverse) {
        blocked_edge_in_path = true;
      }
    }
    if (!blocked_edge_in_path) {
      p_info->blocked_edge = NULL;
    }
  }

  p_info->is_stopping = 0;
  if (p_info->blocked_edge == NULL) {
    set_speed(p_info->saved_speed, loc->train);
  }
  perform_initial_path_actions(loc, p_info);
  handle_path(loc, p_info);
}

void reroute_train(location* loc, path_following_info* p_info) {
  INFO(TRAIN_CONTROLLER, "Train %d being rerouted", loc->train);

  track_edge_array blocked_edges;
  clear_track_edge_array(&blocked_edges);
  reserve_edge(p_info->blocked_edge, &blocked_edges);

  tc_free_all_edges(loc->train, p_info);

  // Usually, we'd just reserve both sides of this location. But since we're rerouting
  // it's most definitely because we can't reserve one side of this location.

  // TODO(f2fung): Enters. Exits. Being blocked both sides.
  tc_reserve_edge(loc->train, &loc->node->edge[DIR_AHEAD], loc, p_info);
  tc_reserve_edge(loc->train, &loc->node->reverse->edge[DIR_AHEAD], loc, p_info);

  p_info->blocked_edge = 0;

  start_route(loc, p_info, &blocked_edges);
}

void check_deadlock(location_array* loc_array, path_following_info* path_info) {
  // Only checks for two trains for now.
  if (loc_array->size != 2) {
    return;
  }

  location* train1 = &loc_array->locations[0];
  location* train2 = &loc_array->locations[1];
  path_following_info* t1_p_info = &path_info[tr_num_to_idx(train1->train)];
  path_following_info* t2_p_info = &path_info[tr_num_to_idx(train2->train)];

  // First make sure that both are stopped.
  if (train1->stopping_distance != 0 || train2->stopping_distance != 0) {
    return;
  }

  /*
   * Reroute if either
   *  1) Both are blocked
   *  2) One is blocked and the other is stopped
   */
  if (t1_p_info->blocked_edge != 0 && t1_p_info->dest != 0 &&
      (t2_p_info->blocked_edge != 0 || t2_p_info->state != ON_ROUTE)) {
    reroute_train(train1, t1_p_info);
  } else if (t2_p_info->blocked_edge != 0 && t2_p_info->dest != 0 &&
             (t1_p_info->blocked_edge != 0 || t1_p_info->state != ON_ROUTE)) {
    reroute_train(train2, t2_p_info);
  }
}

void restart_stopped_trains(location_array* train_locations, path_following_info* path_info) {
  int i;
  for (i = 0; i < train_locations->size; i++) {
    location* cur_loc = &train_locations->locations[i];
    int train = cur_loc->train;
    int train_idx = tr_num_to_idx(train);
    path_following_info* p_info = &path_info[train_idx];
    if (cur_loc->stopping_distance == 0 && p_info->restart_when_stopped) {
      restart_train(cur_loc, p_info);
      p_info->restart_when_stopped = false;
    }
  }
}

void train_controller() {
  RegisterAs("Train Controller");
  location_array train_locations;
  train_locations.size = 0;

  path_following_info path_info[MAX_TRAINS];

  int i, j;
  for (i = 0; i < MAX_TRAINS; i++) {
    path_info[i].path_size = 0;
    path_info[i].path_index = 0;
    path_info[i].blocked_edge = 0;
    path_info[i].is_stopping = 0;
    path_info[i].saved_speed = 0;
    path_info[i].state = NOT_STARTED;
    path_info[i].dest = 0;
    path_info[i].restart_when_stopped = false;
    clear_track_edge_array(&path_info[i].reserved_edges);
    tea_set_name(&path_info[i].reserved_edges, "Some train array");
  }

  // Task waiting on GET_DONE_TRAINS.
  int waiting_tid = 0;

  // Task waiting on NOTIFY_WHEN_TRAINS_STOPPED
  int notify_when_trains_stopped_tid = 0;

  track_edge_array disabled_edges;
  clear_track_edge_array(&disabled_edges);

  int tid, train, train_idx;
  bool occupied_location;
  location* cur_loc;
  track_node* node;
  train_controller_message msg;

  while (1) {
    Receive(&tid, (char *)&msg, sizeof(train_controller_message));
    switch (msg.type) {
      case TRACK_TRAIN: {
        train = msg.user_cmd.train;
        train_idx = tr_num_to_idx(msg.user_cmd.train);

        location loc;
        init_location(&loc);

        set_speed(FINDING_LOCATION_SPEED, train);

        // Find the train amongst the sensors
        sensor_array s_array;
        while (true) {
          s_array.num_sensors = get_sensor_data(s_array.sensors, MAX_NEW_SENSORS);
          node = NULL;

          for (i = 0; i < s_array.num_sensors; i++) {
            sensor* s = &s_array.sensors[i];
            node = get_track_node(sensor2idx(s->group, s->socket));

            // Check if sensor is coming from another sitting train
            occupied_location = false;
            for (j = 0; j < train_locations.size; j++) {
              cur_loc = &train_locations.locations[j];
              if (cur_loc->node == node) {
                occupied_location = true;
                break;
              }
            }

            if (occupied_location) {
              node = NULL;
            } else {
              break;
            }
          }

          if (node != NULL) {
            break;
          }
        }

        set_speed(0, train);

        loc.train = train;
        loc.node = node;
        loc.stopping_distance = 0;
        loc.stopping_time = 0;

        loc.d = FORWARD;
        track_train(train, &loc);
        tracked_trains[train_idx] = 1;

        // Reserve edges we're on.
        // TODO(dzelemba): We could be sitting on 3 nodes here so check for that.
        tc_reserve_edge(train, &loc.node->edge[DIR_AHEAD], &loc, &path_info[train_idx]);
        tc_reserve_edge(train, &loc.node->reverse->edge[DIR_AHEAD], &loc, &path_info[train_idx]);
        Reply(tid, NULL, 0);
        break;
      }
      case SET_ROUTE:
        train = msg.set_route_data.train;
        train_idx = tr_num_to_idx(train);
        Reply(tid, NULL, 0);

        cur_loc = get_train_location(&train_locations, train);
        path_info[train_idx].dest = msg.set_route_data.dest.node;
        path_info[train_idx].saved_speed = msg.set_route_data.speed;

        start_route(cur_loc, &path_info[train_idx], &disabled_edges);
        break;
      case CHANGE_SPEED:
        Reply(tid, NULL, 0);
        set_speed(msg.user_cmd.speed, msg.user_cmd.train);
        break;
      case TR_REVERSE:
        Reply(tid, NULL, 0);
        reverse(msg.user_cmd.train, MAX_STOPPING_TICKS);
        break;
      case UPDATE_LOCATIONS: {
        Reply(tid, NULL, 0);
        memcpy((char *)&train_locations, (const char *)&msg.loc_array, sizeof(location_array));

        for (i = 0; i < train_locations.size; i++) {
          cur_loc = &train_locations.locations[i];
          train = cur_loc->train;
          train_idx = tr_num_to_idx(train);
          if (path_info[train_idx].path_size > 0) {
            handle_path(cur_loc, &path_info[train_idx]);
          }
        }

#ifdef ENABLE_DEADLOCK_DETECTION
        check_deadlock(&train_locations, path_info);
#endif

        check_done_trains(&waiting_tid, &train_locations, path_info);
        check_all_trains_stopped(&notify_when_trains_stopped_tid, &train_locations, path_info);

        restart_stopped_trains(&train_locations, path_info);
        break;
      }
      case RETRY_RESERVATIONS: {
        Reply(tid, NULL, 0);
        for (i = 0; i < msg.tr_array.size; i++) {
          train = msg.tr_array.trains[i];
          path_following_info* p_info = &path_info[tr_num_to_idx(train)];
          if (p_info->blocked_edge != 0) {
            rs_reply reply = rs_reserve(train, p_info->blocked_edge);
            if (reply == SUCCESS) {
              reserve_edge(p_info->blocked_edge, &p_info->reserved_edges);
              restart_train(get_train_location(&train_locations, train), p_info);
            }
          }
        }
        break;
      }
      case TC_GET_DONE_TRAINS:
        waiting_tid = tid;
        check_done_trains(&waiting_tid, &train_locations, path_info);
        break;
      case TC_NOTIFY_WHEN_ALL_TRAINS_STOPPED:
        notify_when_trains_stopped_tid = tid;
        check_all_trains_stopped(&notify_when_trains_stopped_tid, &train_locations, path_info);
        break;
      case TC_DISABLE_EDGE:
        Reply(tid, NULL, 0);
        set_edge(&disabled_edges, msg.edge);
        break;
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
    reversing_tasks[i] = 0;
  }

  init_reservation_server();
  train_controller_tid = Create(MED_PRI, &train_controller);
  Create(MED_PRI, &location_notifier);
  Create(MED_PRI, &rs_notifier);
}

/* Train Methods */

void tr_track(int train) {
  train_controller_message msg;
  msg.type = TRACK_TRAIN;
  msg.user_cmd.train = train;
  Send(train_controller_tid, (char *)&msg, sizeof(train_controller_message), NULL, 0);
}

void tr_set_speed(int speed, int train) {
  train_controller_message msg;
  msg.type = CHANGE_SPEED;
  msg.user_cmd.train = train;
  msg.user_cmd.speed = speed;
  Send(train_controller_tid, (char *)&msg, sizeof(train_controller_message), NULL, 0);
}

void tr_reverse(int train) {
  train_controller_message msg;
  msg.type = TR_REVERSE;
  msg.user_cmd.train = train;
  Send(train_controller_tid, (char *)&msg, sizeof(train_controller_message), NULL, 0);
}

void tr_set_route(int train, int speed, location* loc) {
  train_controller_message msg;
  msg.type = SET_ROUTE;
  msg.set_route_data.train = train;
  msg.set_route_data.speed = speed;
  memcpy((char *)&msg.set_route_data.dest, (const char *)loc, sizeof(location));
  Send(train_controller_tid, (char *)&msg, sizeof(train_controller_message), NULL, 0);
}

void tr_get_done_trains(train_array* tr_array) {
  train_controller_message msg;
  msg.type = TC_GET_DONE_TRAINS;
  Send(train_controller_tid, (char *)&msg, sizeof(train_controller_message),
                             (char *)tr_array, sizeof(train_array));
}

void tr_notify_when_all_trains_stopped(location_array* loc_array) {
  train_controller_message msg;
  msg.type = TC_NOTIFY_WHEN_ALL_TRAINS_STOPPED;
  Send(train_controller_tid, (char *)&msg, sizeof(train_controller_message),
                             (char *)loc_array, sizeof(location_array));
}

void tr_disable_edge(track_edge* edge) {
  train_controller_message msg;
  msg.type = TC_DISABLE_EDGE;
  msg.edge = edge;
  Send(train_controller_tid, (char *)&msg, sizeof(train_controller_message), NULL, 0);
}
