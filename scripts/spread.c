#include "track_data.h"
#include "track_node.h"
#include <stdio.h>

void dfs(track_node* curr, long long int* v, int last, int depth) {
  if (last == -1) {
    return;
  }
  if (v[node2idx(get_track(), curr)] != -1) {
    return;
  }
  if (curr->type == NODE_SENSOR && depth != 0) {
    return;
  }

  v[node2idx(get_track(), curr)] = last;
  for (int i = 0; i < get_num_neighbours(curr->type); i++) {
    dfs(curr->edge[i].dest, v, last, depth + 1);
  }
}

int main() {
  track_node* track = get_track();
  init_tracka();

  long long int v[TRACK_MAX];
  int begin[TRACK_MAX];
  for (int i = 0; i < TRACK_MAX; i++) {
    v[i] = -1;
    begin[i] = -1;
  }

  // This needs to be way more automated

  // TRACK A
  /*begin[sensor2idx('E', 14)] = 516;*/
  /*begin[sensor2idx('E', 10)] = 516;*/
  /*begin[sensor2idx('E', 9)] = 524;*/
  /*begin[sensor2idx('D', 6)] = 524;*/
  /*begin[sensor2idx('D', 5)] = 525;*/
  /*begin[sensor2idx('E', 5)] = 525;*/
  /*begin[sensor2idx('E', 6)] = 543;*/
  /*begin[sensor2idx('D', 3)] = 543;*/

  // TRACK B
  begin[sensor2idx('D', 4)] = 508;
  begin[sensor2idx('D', 3)] = 547;
  begin[sensor2idx('B', 6)] = 539;
  begin[sensor2idx('B', 5)] = 508;
  begin[sensor2idx('B', 16)] = 509;
  begin[sensor2idx('E', 14)] = 509;
  begin[sensor2idx('D', 5)] = 496;
  begin[sensor2idx('E', 6)] = 547;
  begin[sensor2idx('E', 5)] = 496;
  begin[sensor2idx('E', 10)] = 509;
  begin[sensor2idx('C', 9)] = 509;
  begin[sensor2idx('C', 11)] = 539;


  for (int i = 0; i < TRACK_MAX; i++) {
    track_node* node = &(track[i]);
    if (begin[i] != -1) {
      dfs(node, v, begin[i], 0);
    }
  }

  for (int i = 0; i < TRACK_MAX; i++) {
    if (v[i] != -1) {
      printf("_piecewise_velocities[11][%d] = %lld;\n", i, v[i] * 1000000 / 5192361);
    }
  }

  return 0;
}
