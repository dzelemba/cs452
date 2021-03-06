#ifndef __PHYSICS_H__
#define __PHYSICS_H__

#include "location.h"

#define MM_PER_CM 10
#define UM_PER_MM 1000
#define UM_PER_CM UM_PER_MM * MM_PER_CM
#define NM_PER_UM 1000
#define NM_PER_MM NM_PER_UM * UM_PER_MM
#define NM_PER_CM NM_PER_MM * MM_PER_CM

#define DEFAULT_NM_PER_TICK 5311245
#define DEFAULT_ACCELERATING_TICKS 428
#define DEFAULT_STOPPING_DISTANCE 700

// TODO(f2fung): This does nothing right now for want of a deceleration model
#define DEFAULT_STOPPING_TICKS 400

// Ticks
#define MAX_STOPPING_TICKS 400

unsigned int accelerate(int train, unsigned int v0, unsigned int v1, int t);
unsigned int piecewise_velocity(int train, int speed, location* loc);
unsigned int mean_velocity(int train, int speed);
unsigned int stopping_distance(int train, unsigned int v);

void init_physicsa();
void init_physicsb();

#endif
