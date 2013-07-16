#ifndef __TRACK_EDGE_ARRAY_H__
#define __TRACK_EDGE_ARRAY_H__

// In its own file to avoid circular dependancy.

#include "track_data.h"
#include "track_node.h"

typedef struct track_edge_array {
  int map[TRACK_MAX * 2];
} track_edge_array;

void clear_track_edge_array(track_edge_array* t);

int isset_edge(track_edge_array* t, track_edge* edge);

void set_edge(track_edge_array* t, track_edge* edge);

void unset_edge(track_edge_array* t, track_edge* edge);



#endif
