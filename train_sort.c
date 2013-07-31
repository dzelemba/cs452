#include "debug.h"
#include "location.h"
#include "location_server.h"
#include "ourio.h"
#include "sensor.h"
#include "syscall.h"
#include "track_data.h"
#include "train.h"
#include "train_sort.h"

#define MAX_SORTABLE 4

void send_train(int train, location* dest) {
  if (train != 0) {
    tr_set_route(train, DEFAULT_TRAIN_SPEED, dest);
  }
}

void sort_trains(int t1, int t2, int t3, int t4) {
  location dest;
  init_location(&dest);

  tr_disable_edge(string_to_edge("M8", "B9"));
  tr_disable_edge(string_to_edge("M9", "B8"));

  int desired_order[MAX_SORTABLE];

  int num_trains = 0;
  if (t1 != 0) {
    tr_track(t1);
    desired_order[0] = t1;
    num_trains++;
  }
  if (t2 != 0) {
    Delay(200);
    tr_track(t2);
    desired_order[1] = t2;
    num_trains++;
  }
  if (t3 != 0) {
    Delay(200);
    tr_track(t3);
    desired_order[2] = t3;
    num_trains++;
  }
  if (t4 != 0) {
    Delay(200);
    tr_track(t4);
    desired_order[3] = t4;
    num_trains++;
  }

  location_array trains;

  while (true) {
    tr_notify_when_all_trains_stopped(&trains);
    if (trains.size == num_trains) {
      break;
    }
  }

  // All the important spots to the algorithm. These should be defined physical LEFT-TO-RIGHT

  // Where they'll be finally sorted. This should be the inner-loop bottom
  track_node* initial_spots[MAX_SORTABLE];

  // TODO(f2fung): Trains need to be setup very particularly. If there are only
  // 2 trains, they must be put in the last two spots (D13, E13). This shouldn't
  // have to be true.
  initial_spots[0] = get_track_node(sensor2idx('C', 9));
  initial_spots[1] = get_track_node(sensor2idx('B', 2));
  initial_spots[2] = get_track_node(sensor2idx('D', 13));
  initial_spots[3] = get_track_node(sensor2idx('E', 13));

  track_node* final_spots[MAX_SORTABLE];
  final_spots[0] = get_track_node(sensor2idx('C', 9))->reverse;
  final_spots[1] = get_track_node(sensor2idx('B', 2))->reverse;
  final_spots[2] = get_track_node(sensor2idx('D', 13))->reverse;
  final_spots[3] = get_track_node(sensor2idx('E', 13))->reverse;

  // When we have (n - k) remaining unsorted trains, we join them together on
  // the outer-loop bottom.
  track_node* unsorted_spots[MAX_SORTABLE];
  unsorted_spots[0] = get_track_node(sensor2idx('C', 5));
  unsorted_spots[1] = get_track_node(sensor2idx('C', 15));
  unsorted_spots[2] = get_track_node(sensor2idx('D', 12));
  unsorted_spots[3] = get_track_node(sensor2idx('E', 11));

  track_node* prefix_spots[MAX_SORTABLE]; // Outer-loop top
  prefix_spots[0] = NULL;
  prefix_spots[1] = get_track_node(sensor2idx('C', 13));
  prefix_spots[2] = get_track_node(sensor2idx('E', 7));
  prefix_spots[3] = get_track_node(sensor2idx('D', 7));

  track_node* suffix_spots[MAX_SORTABLE]; // Inner-loop top
  suffix_spots[0] = NULL;
  suffix_spots[1] = get_track_node(sensor2idx('B', 5));
  suffix_spots[2] = get_track_node(sensor2idx('D', 3));
  suffix_spots[3] = get_track_node(sensor2idx('E', 5));

  int current_order[MAX_SORTABLE];
  int i, j;
  for (i = 0; i < MAX_SORTABLE; i++) {
    current_order[i] = -1;
  }

  int leftmost_start = MAX_SORTABLE - num_trains;
  for (i = 0; i < num_trains; i++) {
    for (j = 0; j < num_trains; j++) {
      if (trains.locations[i].node == initial_spots[leftmost_start + j]) {
        current_order[j] = trains.locations[i].train;
        break;
      }
    }
  }

  INFO(SORT, "Current order: [ %d, %d, %d, %d ]", current_order[0], current_order[1], current_order[2], current_order[3]);
  INFO(SORT, "Press any key to continue sorting...");
  char unpause = Getc(COM2);
  INFO(SORT, "Begin sorting.");

  int num_finished_sorting = 0;
  for (i = num_trains - 1; i >= 0; i--) {
    if (current_order[i] == desired_order[i]) {
      num_finished_sorting++;
    } else {
      break;
    }
  }

  location loc;

  while (num_finished_sorting < num_trains) {
    INFO(SORT, "Current order: [ %d, %d, %d, %d ]", current_order[0], current_order[1], current_order[2], current_order[3]);

    int next_spot_to_place = MAX_SORTABLE - num_finished_sorting - 1;
    int num_remaining_sorting = num_trains - num_finished_sorting;
    int next_train_to_place = desired_order[num_remaining_sorting - 1];

    INFO(SORT, "Next train to place: %d", next_train_to_place);

    int current_train_spot = -1;
    for (i = 0; i < num_remaining_sorting; i++) {
      if (current_order[i] == next_train_to_place) {
        current_train_spot = i;
        break;
      }
    }

    // This is the prefix
    for (i = 0; i <= current_train_spot; i++) {
      loc.node = prefix_spots[MAX_SORTABLE - i - 1];
      INFO(SORT, "Train %d to %s", current_order[i], loc.node->name);
      tr_set_route(current_order[i], 11, &loc);
    }

    // This is the suffix.
    for (i = current_train_spot + 1; i < num_remaining_sorting; i++) {
      j = i - (current_train_spot + 1);
      loc.node = suffix_spots[MAX_SORTABLE - j - 1];
      INFO(SORT, "Train %d to %s", current_order[i], loc.node->name);
      tr_set_route(current_order[i], 11, &loc);
    }

    tr_notify_when_all_trains_stopped(&trains);

    // Our desired train is now free.
    loc.node = final_spots[next_spot_to_place];
    INFO(SORT, "Train %d to %s", next_train_to_place, loc.node->name);
    tr_set_route(next_train_to_place, 11, &loc);

    tr_notify_when_all_trains_stopped(&trains);
    tr_free_edges_for_train(next_train_to_place);

    // Return the suffix.
    j = MAX_SORTABLE - 1 - (num_finished_sorting + 1);
    for (i = num_remaining_sorting - 1; i >= current_train_spot + 1; i--) {
      loc.node = final_spots[j];
      INFO(SORT, "Train %d to %s", current_order[i], loc.node->name);
      tr_set_route(current_order[i], 11, &loc);
      j--;
    }

    tr_notify_when_all_trains_stopped(&trains);

    // Return the prefix.
    for (i = current_train_spot - 1; i >= 0; i--) {
      loc.node = final_spots[j];
      INFO(SORT, "Train %d to %s", current_order[i], loc.node->name);
      tr_set_route(current_order[i], 11, &loc);
      j--;
    }

    tr_notify_when_all_trains_stopped(&trains);

    // Update the current order
    for (i = current_train_spot; i < num_remaining_sorting - 1; i++) {
      current_order[i] = current_order[i + 1];
    }
    for (i = num_remaining_sorting - 1; i < MAX_SORTABLE; i++) {
      current_order[i] = -1;
    }
    num_finished_sorting++;

    for (i = num_trains - num_finished_sorting - 1; i >= 0; i--) {
      if (current_order[i] == desired_order[i]) {
        num_finished_sorting++;
      } else {
        break;
      }
    }
  }
}
