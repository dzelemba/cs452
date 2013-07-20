#include "debug.h"
#include "track_data.h"
#include "track_edge_array.h"
#include "track_node.h"

#ifdef UNIT
#include "stdio.h"
#endif

int edge2idx(track_edge* edge) {
  track_node* src = edge->src;
  int base = node2idx(get_track(), src) * 2;

  if (src->type == NODE_BRANCH && &src->edge[1] == edge) {
    return base + 1;
  } else {
    return base;
  }
}

track_edge* idx2edge(int idx) {
  track_node* track = get_track();
  int source_id = idx / 2;
  int direction = idx % 2;

  track_node* src = get_track_node(source_id);
  ASSERT(get_num_neighbours(src->type) > direction, "track_edge_array.c: idx2edge: Invalid index %d", idx);
  return &src->edge[direction];
}

void tea_set_name(track_edge_array* t, char* name) {
  t->name = name;
}

void clear_track_edge_array(track_edge_array* t) {
  int i;
  for (i = 0; i < EDGE_MAX; i++) {
    t->map[i] = 0;
  }
}

bool isset_edge(track_edge_array* t, track_edge* edge) {
  return t->map[edge2idx(edge)];
}

void set_edge(track_edge_array* t, track_edge* edge) {
  t->map[edge2idx(edge)] = true;
}

void unset_edge(track_edge_array* t, track_edge* edge) {
  t->map[edge2idx(edge)] = false;
}

void set_edge_value(track_edge_array* t, track_edge* edge, int val) {
  t->map[edge2idx(edge)] = val;
}

int get_edge_value(track_edge_array* t, track_edge* edge) {
  return t->map[edge2idx(edge)];
}
