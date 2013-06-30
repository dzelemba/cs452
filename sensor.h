#ifndef __SENSOR_H__
#define __SENSOR_H__

/*
 * Sensor
 */

#define NUM_GROUPS 5
#define SOCKETS_PER_GROUP 16

#define NUM_SENSORS (NUM_GROUPS * SOCKETS_PER_GROUP)

// TODO(dzelemba): Find a good way to pass variable sized data.
#define MAX_NEW_SENSORS 4

typedef struct sensor {
  char group;
  int socket;
} sensor;

int sensor_to_int(sensor* s);

void int_to_sensor(int i, sensor* s);

#endif
