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
