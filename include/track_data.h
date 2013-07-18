#ifndef __TRACK_DATA_H__
#define __TRACK_DATA_H__

#include "track_edge_array.h"
#include "track_node.h"

track_node* get_track();
track_edge_array* get_broken_edges();

void init_tracka(track_node *track);
void init_trackb(track_node *track);

int get_track_number();

#endif
