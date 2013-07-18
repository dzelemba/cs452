#ifndef __LOCATION_SERVER_H__
#define __LOCATION_SERVER_H__

#include "location.h"
#include "track_node.h"
#include "train.h"

typedef struct location_array {
  location locations[MAX_TRAINS];
  int size;
} location_array;

int get_track_index(track_node* track, location* loc);

location* get_train_location(location_array* loc_array, int train);

/*
 * Location Server
 */

void start_location_server();

void track_train(int train, location* loc);

void ls_set_direction(int train, direction dir);

void get_location_updates(location_array* loc_array);

void ls_train_reversed(int train);

char* direction_to_string(direction d);

track_edge* get_next_edge(track_node* node);

track_node* get_next_sensor(track_edge* edge);

int get_next_sensor_distance(track_node* node);

void ds_update_speed(int train, int speed);
#endif
