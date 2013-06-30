#ifndef __SENSOR_SERVER_H__
#define __SENSOR_SERVER_H__

#include "sensor.h"

void start_sensor_server();

int get_sensor_data(sensor* sensors, int max_sensors);

#endif
