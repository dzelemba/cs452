#include "track_node.h"
#include "track_data.h"
#include "physics.h"

static int _exp_velocities[TRACK_MAX];

int* get_exp_velocities() {
  return _exp_velocities;
}

int get_exp_velocity_at_location(int loc_id) {
  return _exp_velocities[loc_id];
}

void init_physicsa(int *exp_velocities) {
  int i;
  for (i = 0; i < TRACK_MAX; i++) {
    exp_velocities[i] = DEFAULT_UM_PER_TICK;
  }

  exp_velocities[50] = 5430;
  exp_velocities[52] = 5250;
  exp_velocities[53] = 5240;
  exp_velocities[68] = 5250;
  exp_velocities[69] = 5430;
  exp_velocities[72] = 5240;
  exp_velocities[73] = 5160;
  exp_velocities[77] = 5160;
  exp_velocities[94] = 5240;
  exp_velocities[95] = 5240;
  exp_velocities[96] = 5240;
  exp_velocities[97] = 5240;
  exp_velocities[98] = 5430;
  exp_velocities[99] = 5430;
}

void init_physicsb(int *exp_velocities) {
  // TODO(f2fung): I've literally done no experiments on Track B
  int i;
  for (i = 0; i < TRACK_MAX; i++) {
    exp_velocities[i] = DEFAULT_UM_PER_TICK;
  }
}
