#include "dijkstra.h"
#include "heapplus.h"
#include "track_data.h"
#include "track_edge_array.h"
#include "track_node.h"

#ifdef UNIT
#include <stdio.h>
#endif

#define ENABLE_REVERSE 1

// At a full speed of 50cm/s stopping time is about 4s
// and it takes another 4s to get back to full speed,
// so a reverse costs us about 8 * 50 = 400cm = 4000mm
// TODO(dzelemba): Make this dependant on current train speed.
#define REVERSE_COST 4000

static heap_node dijkstra_mem[TRACK_MAX];
static int dijkstra_dict[TRACK_MAX];
static heapplus path_heap;

static sequence path[TRACK_MAX];
static char visited[TRACK_MAX];

int get_path(track_node* track, track_node* src, track_node* dest, track_edge_array* blocked_edges,
             sequence* out_path, int* out_size) {
  return get_path_from_idx(track, node2idx(track, src), node2idx(track, dest), blocked_edges, out_path, out_size);
}

// Perfectly thread unsafe
int get_path_debug(track_node* track, char src_sensor, int src_socket,
                    char dest_sensor, int dest_socket, track_edge_array* blocked_edges,
                    sequence* out_path, int* out_size) {
  int src = sensor2idx(src_sensor, src_socket);
  int dest = sensor2idx(dest_sensor, dest_socket);
  return get_path_from_idx(track, src, dest, blocked_edges, out_path, out_size);
}

int get_path_from_idx(track_node* track, int src, int dest, track_edge_array* blocked_edges, sequence* out_path, int* out_size) {
  track_edge_array* broken_edges = get_broken_edges();

  init_heapplus(&path_heap, dijkstra_mem, dijkstra_dict, TRACK_MAX);

  int i;
  for (i = 0; i < TRACK_MAX; i++) {
    visited[i] = 0;
  }

  path[src] = (sequence) { src, DO_NOTHING, 0 };
  heapplus_insert(&path_heap, 0, src);

  int first_rev = node2idx(get_track(), get_track_node(src)->reverse);
  path[first_rev] = (sequence) { src, REVERSE, 0 };
  heapplus_insert(&path_heap, 0, first_rev);

  int ret = 0;
  while (heapplus_size(&path_heap) > 0) {
    int c = heapplus_min_pri(&path_heap);
    int v = heapplus_delete_min(&path_heap);
    /*printf("from %s, @ %s\n", track[path[v].location].name, track[v].name);*/

    visited[v] = 1;
    if (v == dest) {
      ret = c;
      break;
    }

    track_node* node = &(track[v]);
    int num_neighbours = get_num_neighbours(node->type);

    i = 0;
    for (i = 0; i < num_neighbours; i++) {
      if (isset_edge(broken_edges, &node->edge[i]) || ((blocked_edges != NULL) && isset_edge(blocked_edges, &node->edge[i]))) {
        continue;
      }

      int next = node2idx(track, node->edge[i].dest);
      if (visited[next] == 0 && heapplus_insert(&path_heap, c + node->edge[i].dist, next)) {
        if (node->type == NODE_BRANCH) {
          path[next] = (sequence) { v, (i == DIR_STRAIGHT) ? TAKE_STRAIGHT : TAKE_CURVE, 0 };
        } else {
          path[next] = (sequence) { v, DO_NOTHING, 0 };
        }
      }
    }

#ifdef ENABLE_REVERSE
    // TODO: Maybe we need to do better than just reverse at sensors
    if (node->type == NODE_SENSOR || node2idx(track, node) == src) {
      int rev = node2idx(track, node->reverse);
      if (visited[rev] == 0 && heapplus_insert(&path_heap, c + REVERSE_COST, rev)) {
        path[rev] = (sequence) { v, REVERSE, 0 };
      }
    }
#endif
  }

  if (visited[dest] == 0) {
    return -1;
  }

  // Backtrack the path
  out_path[0] = (sequence) { dest, DO_NOTHING, 0 };

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

  return ret;
}
