#ifndef __DISTANCE_SERVER_H__
#define __DISTANCE_SERVER_H__

void start_distance_server();

void ds_track_train(int train);

void ds_update_speed(int train, int speed);

void ds_get_update(int* train, int* dx, int* stopping_distance, int* stopping_time);

#endif
