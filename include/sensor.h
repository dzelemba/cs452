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

typedef struct sensor_array {
  sensor sensors[MAX_NEW_SENSORS];
  int num_sensors;
} sensor_array;

int sensor_to_int(sensor* s);

void int_to_sensor(int i, sensor* s);

int sensor_equal(sensor* s1, sensor* s2);

void sensor_copy(sensor* destination, sensor* source);

#endif
