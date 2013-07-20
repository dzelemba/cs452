#ifndef __TRACK_DATA_H__
#define __TRACK_DATA_H__

#include "track_edge_array.h"
#include "track_node.h"

track_node* get_track();
track_node* get_track_node(int idx);

track_edge_array* get_broken_edges();

void init_tracka();
void init_trackb();

int get_track_number();

#endif
