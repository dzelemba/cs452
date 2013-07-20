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

}

void tea_set_name(track_edge_array* t, char* name) {
  t->name = name;
}

void clear_track_edge_array(track_edge_array* t) {
  int i;
  for (i = 0; i < TRACK_MAX * 2; i++) {
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
