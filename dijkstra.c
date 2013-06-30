#include "dijkstra.h"
#include "heapplus.h"
#include "track_data.h"
#include "track_node.h"

#ifdef UNIT
#include <stdio.h>
#endif

// TODO: Come up with a better path cost model
#define REVERSE_COST 500

// We redefine this so we can run this code in a unittest
#define SOCKETS_PER_SENSOR 16

static int NUM_NEIGHBOURS[NUM_NODE_TYPES] = { 0, 1, 2, 1, 1, 0 };

static heap_node dijkstra_mem[TRACK_MAX];
static int dijkstra_dict[TRACK_MAX];
static heapplus path_heap;

static sequence path[TRACK_MAX];
static char visited[TRACK_MAX];

int sensor2idx(char sensor, int socket) {
  return (sensor - 'A') * SOCKETS_PER_SENSOR + (socket - 1);
}

int node2idx(track_node* track, track_node* node) {
  return (node - track);
}

// Perfectly thread unsafe
void get_path_debug(track_node* track, char src_sensor, int src_socket,
                    char dest_sensor, int dest_socket, sequence* out_path,
                    int* out_size) {
  int src = sensor2idx(src_sensor, src_socket);
  int dest = sensor2idx(dest_sensor, dest_socket);
  get_path(track, src, dest, out_path, out_size);
}

void get_path(track_node* track, int src, int dest, sequence* out_path, int* out_size) {
  init_heapplus(&path_heap, dijkstra_mem, dijkstra_dict, TRACK_MAX);

  int i;
  for (i = 0; i < TRACK_MAX; i++) {
    visited[i] = 0;
  }

  path[src] = (sequence) { src, DO_NOTHING };
  heapplus_insert(&path_heap, 0, src);

  while (heapplus_size(&path_heap) > 0) {
    int c = heapplus_min_pri(&path_heap);
    int v = heapplus_delete_min(&path_heap);
    /*printf("from %s, @ %s\n", track[path[v].location].name, track[v].name);*/

    visited[v] = 1;
    if (v == dest) {
      break;
    }

    track_node* node = &(track[v]);
    int num_neighbours = NUM_NEIGHBOURS[node->type];

    i = 0;
    for (i = 0; i < num_neighbours; i++) {
      int next = node2idx(track, node->edge[i].dest);
      if (visited[next] == 0 && heapplus_insert(&path_heap, c + node->edge[i].dist, next)) {
        if (node->type == NODE_BRANCH) {
          path[next] = (sequence) { v, (i == DIR_STRAIGHT) ? TAKE_STRAIGHT : TAKE_CURVE };
        } else {
          path[next] = (sequence) { v, DO_NOTHING };
        }
      }
    }

    // TODO: Maybe we need to do better than just reverse at sensors
    if (node->type == NODE_SENSOR) {
      int rev = node2idx(track, node->reverse);
      if (visited[rev] == 0 && heapplus_insert(&path_heap, c + REVERSE_COST, rev)) {
        path[rev] = (sequence) { v, REVERSE };
      }
    }
  }

  // Backtrack the path
  out_path[0] = (sequence) { dest, DO_NOTHING };

  i = 1;
  int it = dest;
  while (path[it].location != it) {
    out_path[i] = path[it];
    it = path[it].location;
    i++;
  }

  // Reverse the sequence to get the correct forward actions
  int pathlen = i;
  *out_size = pathlen;
  for (i = 0; i < pathlen / 2; i++) {
    sequence swap = out_path[i];
    out_path[i] = out_path[pathlen - i - 1];
    out_path[pathlen - i - 1] = swap;
  }
}
