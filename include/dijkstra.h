#ifndef __DIJKSTRA_H__
#define __DIJKSTRA_H__

#include "location_server.h"
#include "track_edge_array.h"
#include "track_node.h"

typedef enum {
  DO_NOTHING,
  REVERSE,
  TAKE_STRAIGHT,
  TAKE_CURVE
} sequence_action;

typedef struct sequence {
  short location; // Matches track_node ids
  sequence_action action;
  int performed_action;
} sequence;

int get_path(track_node* track, track_node* src, track_node* dest, track_edge_array* blocked_edges,
             sequence* out_path, int* out_size);

int get_path_debug(track_node* track, char src_sensor, int src_socket,
                   char dest_sensor, int dest_socket, track_edge_array* blocked_edges,
                   sequence* out_path, int* out_size);

int get_path_from_idx(track_node* track, int src, int dest, track_edge_array* blocked_edges, sequence* out_path, int* out_size);

#endif
