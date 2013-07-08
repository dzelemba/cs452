#ifndef __PHYSICS_H__
#define __PHYSICS_H__

#include "location.h"

#define DEFAULT_NM_PER_TICK 5330000
#define DEFAULT_ACCELERATING_TICKS 428
#define DEFAULT_STOPPING_DISTANCE 700
#define DEFAULT_STOPPING_TICKS 200

unsigned int accelerate(int train, unsigned int v0, unsigned int v1, int t);
unsigned int stop(int train, unsigned int v, int t);
unsigned int ticks_to_accelerate(unsigned int v0, unsigned int v1);
unsigned int piecewise_velocity(int train, int speed, location* loc);
unsigned int mean_velocity(int train, int speed);
unsigned int stopping_distance(int train, unsigned int v);
unsigned int stopping_time(int train, unsigned int v);

void init_physicsa();
void init_physicsb();

#endif
