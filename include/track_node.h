#ifndef __TRACK_NODE_H__
#define __TRACK_NODE_H__

#include "sensor.h"

typedef enum {
  NODE_NONE,
  NODE_SENSOR,
  NODE_BRANCH,
  NODE_MERGE,
  NODE_ENTER,
  NODE_EXIT,
  NUM_NODE_TYPES
} node_type;

#define DIR_AHEAD 0
#define DIR_STRAIGHT 0
#define DIR_CURVED 1

struct track_node;
typedef struct track_node track_node;
typedef struct track_edge track_edge;

struct track_edge {
  track_edge *reverse;
  track_node *src, *dest;
  int dist;             /* in millimetres */
};

struct track_node {
  const char *name;
  node_type type;
  int num;              /* sensor or switch number */
  track_node *reverse;  /* same location, but opposite direction */
  track_edge edge[2];
};

track_node* get_track_node(track_node* track, int idx);

int get_num_neighbours(node_type type);

int enter2idx(int num);

int exit2idx(int num);

int merge2idx(int num);

int branch2idx(int num);

int sensor2idx(char sensor, int socket);

int node2idx(track_node* track, track_node* node);

void node2sensor(track_node* track, track_node* node, sensor* s);

void get_next_sensors(track_node* track, track_node* node, sensor* sensors, int* num_sensors);

track_edge* get_next_edge(track_node* node);

track_node* get_next_sensor(track_node* node);

#endif
