#include "track_node.h"
#include "debug.h"

// We redefine this so we can run this code in a unittest
#define SOCKETS_PER_SENSOR 16

static int NUM_NEIGHBOURS[NUM_NODE_TYPES] = { 0, 1, 2, 1, 1, 0 };

int get_num_neighbours(node_type type) {
  return NUM_NEIGHBOURS[type];
}

int sensor2idx(char sensor, int socket) {
  return (sensor - 'A') * SOCKETS_PER_SENSOR + (socket - 1);
}

int node2idx(track_node* track, track_node* node) {
  return (node - track);
}

void node2sensor(track_node* track, track_node* node, sensor* s) {
  int idx = node2idx(track, node);
  s->group = (idx / SOCKETS_PER_SENSOR) + 'A';
  s->socket = (idx % SOCKETS_PER_SENSOR) + 1;
}

// Forward declare as the next two functions call each other.
void get_next_sensors_recurse(track_node* track, track_node* node, sensor* sensors, int* num_sensors);

void get_next_sensors_worker(track_node* track, track_node* node, sensor* sensors, int* num_sensors) {
  // Base Case
  if (node->type == NODE_SENSOR) {
    node2sensor(track, node, &sensors[*num_sensors]);
    *num_sensors = *num_sensors + 1;
    return;
  }

  get_next_sensors_recurse(track, node, sensors, num_sensors);
}

void get_next_sensors_recurse(track_node* track, track_node* node, sensor* sensors, int* num_sensors) {
  int i;
  for (i = 0; i < get_num_neighbours(node->type); i++) {
    get_next_sensors_worker(track, node->edge[i].dest, sensors, num_sensors);
  }
}

void get_next_sensors(track_node* track, track_node* node, sensor* sensors, int* num_sensors) {
  ASSERT(node->type == NODE_SENSOR, "track_node.c: get_next_sensors");

  get_next_sensors_recurse(track, node, sensors, num_sensors);
}


