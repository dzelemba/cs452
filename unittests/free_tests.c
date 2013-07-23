#include "location.h"
#include "physics.h"
#include "track_data.h"
#include "track_edge_array.h"
#include "track_node.h"
#include "train.h"

#define TRAIN_LENGTH 215

#include <stdio.h>

void previous_edges_within_distance(track_edge* curr, int cd, int fd, track_edge_array* seen) {
  printf("prev_edges %s -> %s, %d, %d\n", curr->src->name, curr->dest->name, cd, fd);

  int i;
  track_node* dest = curr->dest;

  if (cd >= fd) {
    set_edge(seen, curr);
    if (cd >= fd + 2 * TRAIN_LENGTH) {
      return;
    }
  }

  cd += curr->dist;
  for (i = 0; i < get_num_neighbours(dest->type); i++) {
    previous_edges_within_distance(&dest->edge[i], cd, fd, seen);
  }
}

void tc_free_previous_edges(int train, location* cur_loc, int distance_behind) {
  track_edge_array freeable_edges;
  clear_track_edge_array(&freeable_edges);

  track_node* rev = cur_loc->node->reverse;
  track_node* node;

  int i, j;

  int cd = cur_loc->um_past_node / UM_PER_MM;

  // Free edges only _after_ the edge we're on
  for (i = 0; i < get_num_neighbours(rev->type); i++) {
    node = rev->edge[i].dest;
    for (j = 0; j < get_num_neighbours(node->type); j++) {
      previous_edges_within_distance(&node->edge[j], cd + rev->edge[i].dist, distance_behind, &freeable_edges);
    }
  }

  for (i = 0; i < TRACK_MAX; i++) {
    node = get_track_node(i);
    for (j = 0; j < get_num_neighbours(node->type); j++) {
      if (isset_edge(&freeable_edges, &node->edge[j])) {
        printf("Want to free %s -> %s\n", node->edge[j].src->name, node->edge[j].dest->name);
        /*tc_free_edge(train, &node->edge[j], p_info);*/
      }
    }
  }
}

void free_tests() {
  printf("******* Free Tests ********\n\n");

  init_tracka();
  location loc;
  init_location(&loc);

  loc.node = get_track_node(sensor2idx('E', 16));
  tc_free_previous_edges(10, &loc, 2 * TRAIN_LENGTH);
}
