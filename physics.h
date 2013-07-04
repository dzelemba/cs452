#ifndef __PHYSICS_H__
#define __PHYSICS_H__

#define DEFAULT_UM_PER_TICK 5330

int* get_exp_velocities();
int get_exp_velocity_at_location(int loc_id);
void init_physicsa(int *exp_velocities);
void init_physicsb(int *exp_velocities);

#endif
