#ifndef __TRACK_DATA_H__
#define __TRACK_DATA_H__

#include "track_edge_array.h"
#include "track_node.h"

#define MAX_EDGE_GROUP_SIZE 8

track_node* get_track();
track_node* get_track_node(int idx);
track_node* get_related_branch(track_node* node);
int get_edge_group(track_edge* edge, track_edge** edge_group);

track_edge_array* get_broken_edges();

void init_tracka();
void init_trackb();

int get_track_number();

#endif
