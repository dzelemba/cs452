#include "demo.h"
#include "syscall.h"
#include "priorities.h"
#include "train.h"
#include "timer.h"
#include "location.h"
#include "track_node.h"
#include "track_data.h"
#include "ourlib.h"
#include "user_prompt.h"
#include "switch_server.h"

#define NUM_IGNORED_NODES 55
// TODO(dzelemba): Add Track B.
static const int const ignored_nodes[NUM_IGNORED_NODES] =
  { /* Track A */
    1, 2, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, /* A1, A2, A5 -> A16 */
    22, 23, 24, 25, 26, 27, /* B7 -> B12 */
    80, 81, 82, 83, 84, 85, 86, 87, 102, 103, /* BR 1, 2, 3, 4, 12 */
    34, 35, 38, 39, /* C3, C4, C7, C8 */
    124, 125, 126, 127, 128, 129, 130, 131, 132, 133,
    134, 135, 136, 137, 138, 139, 140, 141, 142, 143, 144 /* EN/EX */
  };

static int train1, train2;
static track_node* destinations[NUM_TRAINS];

bool check_nodes_equal(track_node* d1, track_node* d2) {
  return d1 == d2 || d1->reverse == d2;
}

bool check_conflicting_nodes(track_node* d1, track_node* d2) {
  return check_nodes_equal(d1, d2) ||
         check_nodes_equal(d1->edge[DIR_AHEAD].dest, d2) ||
         check_nodes_equal(d2->edge[DIR_AHEAD].dest,d1) ||
         (d1->type == NODE_SENSOR && check_nodes_equal(d1->edge[DIR_CURVED].dest, d2)) ||
         (d2->type == NODE_SENSOR && check_nodes_equal(d2->edge[DIR_CURVED].dest, d1));
}

bool check_conflicting_destinations(int train, track_node* dest, track_node** cur_destinations) {
  (void)train; // Silence compiler warnings.

  int i;
  for (i = 0; i < NUM_TRAINS; i++) {
    // TODO(dzelemba): Ignore own location. This is handy for now
    // as it avoids really short hops that we're bad at.
    track_node* other_dest = cur_destinations[i];
    if (other_dest != 0) {
      if (check_conflicting_nodes(dest, other_dest)) {
        return true;
      }
    }
  }

  return false;
}

bool check_ignored_nodes(int dest) {
  int i;
  for (i = 0; i < NUM_IGNORED_NODES; i++) {
    if (dest == ignored_nodes[i]) {
      return true;
    }
  }
  return false;
}

void get_random_destination(int train, location* loc, track_node** cur_destinations) {
  init_location(loc);

  track_node* dest = NULL;
  int rand_dest;
  while(1) {
    rand_dest = rand() % TRACK_MAX;

    dest = get_track_node(rand_dest);
    if (check_ignored_nodes(rand_dest) ||
        check_conflicting_destinations(train, dest, cur_destinations)) {
      continue;
    }
    break;
  }

  loc->node = dest;
  cur_destinations[train] = dest;
}

void demo() {
  int i, train;
  train_array done_trains;
  location next_dest;
  while (1) {
    tr_get_done_trains(&done_trains);
    for (i = 0; i < done_trains.size; i++) {
      train = done_trains.trains[i];
      get_random_destination(train, &next_dest, destinations);
      tr_set_route(train, DEFAULT_TRAIN_SPEED, &next_dest);
      update_demo_location(train == train1 ? 0 : 1, &next_dest);
    }
  }

  Exit();
}

/*
 * Public Methods
 */


void demo_continue() {
  location next_dest;
  get_random_destination(train1, &next_dest, destinations);
  tr_set_route(train1, DEFAULT_TRAIN_SPEED, &next_dest);
  update_demo_location(0, &next_dest);

  get_random_destination(train2, &next_dest, destinations);
  tr_set_route(train2, DEFAULT_TRAIN_SPEED, &next_dest);
  update_demo_location(1, &next_dest);
}

void demo_loop() {
  Create(MED_PRI, &demo);
}

void start_demo(int t1, int t2) {
  train1 = t1;
  train2 = t2;
  init_demo_output(train1, train2);

  int i;
  for (i = 0; i < NUM_TRAINS; i++) {
    destinations[i] = NULL;
  }

  // To avoid going into the bays.
  tr_sw(18, 'C');
  tr_sw(5, 'C');
  tr_sw(11, 'C');

  tr_track(train1);
  Delay(500);
  tr_track(train2);
}
