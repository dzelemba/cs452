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

unsigned int stopping_distance(int train, unsigned int v) {
  return ((DEFAULT_STOPPING_DISTANCE / 5) * v) / DEFAULT_NM_PER_TICK * 5;
}

void init_mean_velocities() {
  int i;
  for (i = 0; i < MAX_TRAINS; i++) {
    _mean_velocities[0][i] = 0;
    _mean_velocities[11][i] = DEFAULT_NM_PER_TICK;
  }

  _mean_velocities[11][tr_num_to_idx(47)] = 5311245;

  _mean_velocities[11][tr_num_to_idx(48)] = 5145417;

  _mean_velocities[8][tr_num_to_idx(49)] =  3612000; // Avi's data
  _mean_velocities[9][tr_num_to_idx(49)] =  4586000; // Avi's data
  _mean_velocities[10][tr_num_to_idx(49)] = 5111000; // Avi's data
  _mean_velocities[11][tr_num_to_idx(49)] = 5683000; // Avi's data
  _mean_velocities[12][tr_num_to_idx(49)] = 6135000; // Avi's data
  _mean_velocities[13][tr_num_to_idx(49)] = 6203000; // Avi's data
  _mean_velocities[14][tr_num_to_idx(49)] = 6320000; // Avi's data

  _mean_velocities[1][tr_num_to_idx(50)] =  89622;
  _mean_velocities[2][tr_num_to_idx(50)] =  705592;
  _mean_velocities[3][tr_num_to_idx(50)] =  1227363;
  _mean_velocities[4][tr_num_to_idx(50)] =  1730769;
  _mean_velocities[5][tr_num_to_idx(50)] =  2193155;
  _mean_velocities[6][tr_num_to_idx(50)] =  2712882;
  _mean_velocities[8][tr_num_to_idx(50)] =  3772000; // Avi's data
  _mean_velocities[9][tr_num_to_idx(50)] =  4287000; // Avi's data
  _mean_velocities[10][tr_num_to_idx(50)] = 4769000; // Avi's data
  _mean_velocities[11][tr_num_to_idx(50)] = 5192361;
  _mean_velocities[12][tr_num_to_idx(50)] = 5664000; // Avi's data
  _mean_velocities[13][tr_num_to_idx(50)] = 5740000; // Avi's data
  _mean_velocities[14][tr_num_to_idx(50)] = 5867000; // Avi's data
}

void init_physicsa() {
  init_mean_velocities();

  int i;
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
  init_mean_velocities();

  int i;
  for (i = 0; i < TRACK_MAX; i++) {
    _piecewise_velocities[0][i] = 100;
    _piecewise_velocities[11][i] = 100;
  }

  _piecewise_velocities[11][20] = 97;
  _piecewise_velocities[11][21] = 103;
  _piecewise_velocities[11][31] = 98;
  _piecewise_velocities[11][40] = 98;
  _piecewise_velocities[11][42] = 103;
  _piecewise_velocities[11][50] = 105;
  _piecewise_velocities[11][51] = 97;
  _piecewise_velocities[11][52] = 95;
  _piecewise_velocities[11][68] = 95;
  _piecewise_velocities[11][69] = 105;
  _piecewise_velocities[11][73] = 98;
  _piecewise_velocities[11][77] = 98;
  _piecewise_velocities[11][98] = 105;
  _piecewise_velocities[11][99] = 105;
  _piecewise_velocities[11][104] = 103;
  _piecewise_velocities[11][105] = 103;
  _piecewise_velocities[11][108] = 98;
  _piecewise_velocities[11][109] = 98;
}
