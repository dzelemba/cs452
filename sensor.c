#include "sensor.h"

/*
 * Sensor
 */

int sensor_to_int(sensor* s) {
  return (s->group - 'A') * SOCKETS_PER_GROUP + s->socket;
}

void int_to_sensor(int i, sensor* s) {
  s->group = (i / SOCKETS_PER_GROUP) + 'A';
  s->socket = i % SOCKETS_PER_GROUP;
}

int sensor_equal(sensor* s1, sensor* s2) {
  return (s1->group == s2->group) && (s1->socket == s2->socket);
}

void sensor_copy(sensor* destination, sensor* source) {
  destination->group = source->group;
  destination->socket = source->socket;
}

