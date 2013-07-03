#ifndef __DISTANCE_SERVER_H__
#define __DISTANCE_SERVER_H__

void start_distance_server();

void ds_track_train(int train);

void ds_update_speed(int train, int speed);

/*
 * Responds after any train that the train server
 * is tracking has moved 1cm. The train that has moved
 * will be in 'train'.
 */
void ds_get_update(int* train, int* dx);

#endif
