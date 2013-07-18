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

#define DEFAULT_TRAIN_SPEED 11

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

void get_random_destination(location* loc) {
  init_location(loc);

  int rand_dest, i;
  while(1) {
    int done = 1;
    rand_dest = rand() % TRACK_MAX;
    for (i = 0; i < NUM_IGNORED_NODES; i++) {
      if (rand_dest == ignored_nodes[i]) {
        done = 0;
        break;
      }
    }
    if (done) {
      break;
    }
  }

  loc->node = get_track_node(get_track(), rand_dest);
}

void demo() {
  int i, train;
  train_array done_trains;
  location next_dest;
  while (1) {
    tr_get_done_trains(&done_trains);
    for (i = 0; i < done_trains.size; i++) {
      train = done_trains.trains[i];
      get_random_destination(&next_dest);
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
  get_random_destination(&next_dest);
  tr_set_route(train1, DEFAULT_TRAIN_SPEED, &next_dest);
  update_demo_location(0, &next_dest);

  get_random_destination(&next_dest);
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

  // To avoid going into the bays.
  tr_sw(18, 'C');
  tr_sw(5, 'C');

  tr_track(train1);
  Delay(500);
  tr_track(train2);
}
