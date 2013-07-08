#include "location.h"
#include "ourlib.h"
#include "physics.h"
#include "track_data.h"
#include "track_node.h"
#include "train.h"

static unsigned int _piecewise_velocities[NUM_SPEEDS + 1][TRACK_MAX];
static unsigned int _mean_velocities[NUM_SPEEDS + 1][MAX_TRAINS];

unsigned int accelerate(int train, unsigned int v0, unsigned int v1, int t) {
  // TODO(f2fung): Scale this by |v0 - v1|
  int at_poly = max(-6 * t * t + 3463 * t - 279770, 0); // Our polyfit model
  v0 += (at_poly / 10);
  return (v0 < v1) ? v0 : v1;
}

unsigned int stop(int train, unsigned int v, int t) {
  unsigned int dv = DEFAULT_NM_PER_TICK / DEFAULT_STOPPING_TICKS;
  if (v < dv) {
    return 0;
  } else {
    return v - dv;
  }
}

unsigned int ticks_to_accelerate(unsigned int v0, unsigned int v1) {
  unsigned int dv = (v0 < v1) ? v1 - v0 : v0 - v1;
  // TODO(f2fung): This is simple scaling and isn't backed by any experimental data
  return dv * DEFAULT_ACCELERATING_TICKS / DEFAULT_NM_PER_TICK;
}

unsigned int piecewise_velocity(int train, int speed, location* loc) {
  return _piecewise_velocities[speed][node2idx(get_track(), loc->node)];
}

unsigned int mean_velocity(int train, int speed) {
  return _mean_velocities[speed][tr_num_to_idx(train)];
}

// TODO(f2fung): Stopping model is really crude
unsigned int stopping_distance(int train, unsigned int v) {
  return DEFAULT_STOPPING_DISTANCE * v / DEFAULT_NM_PER_TICK;
}

unsigned int stopping_time(int train, unsigned int v) {
  return DEFAULT_ACCELERATING_TICKS * v / DEFAULT_NM_PER_TICK;
}

void init_physicsa() {
  int i;
  for (i = 0; i < MAX_TRAINS; i++) {
    _mean_velocities[0][i] = 0;
    _mean_velocities[11][i] = DEFAULT_NM_PER_TICK;
  }

  for (i = 0; i < TRACK_MAX; i++) {
    _piecewise_velocities[0][i] = 100;
    _piecewise_velocities[11][i] = 100;
  }

  _piecewise_velocities[11][50] = 102;
  _piecewise_velocities[11][52] = 98;
  _piecewise_velocities[11][53] = 98;
  _piecewise_velocities[11][68] = 98;
  _piecewise_velocities[11][69] = 102;
  _piecewise_velocities[11][72] = 98;
  _piecewise_velocities[11][73] = 97;
  _piecewise_velocities[11][77] = 97;
  _piecewise_velocities[11][94] = 98;
  _piecewise_velocities[11][95] = 98;
  _piecewise_velocities[11][96] = 98;
  _piecewise_velocities[11][97] = 98;
  _piecewise_velocities[11][98] = 102;
  _piecewise_velocities[11][99] = 102;
}

void init_physicsb() {
  // TODO(f2fung): I've literally done no experiments on Track B
  int i;
  for (i = 0; i < MAX_TRAINS; i++) {
    _mean_velocities[0][i] = 0;
    _mean_velocities[11][i] = DEFAULT_NM_PER_TICK;
  }

  for (i = 0; i < TRACK_MAX; i++) {
    _piecewise_velocities[0][i] = 100;
    _piecewise_velocities[11][i] = 100;
  }
}
