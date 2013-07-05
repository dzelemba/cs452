#include "calibration.h"
#include "distance_server.h"
#include "location_server.h"
#include "ourio.h"
#include "sensor.h"
#include "sensor_server.h"
#include "syscall.h"
#include "track_data.h"
#include "track_node.h"
#include "train.h"

// Table of sensor to sensor distances
#define UNKNOWN_DISTANCE -1
#define IX(u, v) sensor2idx(u, v)
#define NUM_SAMPLES 10

static int known_distances[80][80];
static int times_seen[80][80];
static int recorded_times[80][80][NUM_SAMPLES];
static int last_seen[80];

int average_recorded(int src, int dest) {
  int sum = 0;
  int i;
  for (i = 0; i < NUM_SAMPLES; i++) {
    sum += recorded_times[src][dest][i];
  }
  return (sum * 10) / NUM_SAMPLES; // TODO: Better precision
}

int variance(int src, int dest, int mean) {
  int sum = 0;
  int i;
  for (i = 0; i < NUM_SAMPLES; i++) {
    int d = recorded_times[src][dest][i] * 10 - mean;
    sum += d * d;
  }
  return sum / NUM_SAMPLES;
}

void velocity_calibration() {
  track_node* track = get_track();
  init_tracka(track);

  int i, j;
  for (i = 0; i < 80; i++) {
    for (j = 0; j < 80; j++) {
      known_distances[i][j] = UNKNOWN_DISTANCE;
      times_seen[i][j] = 0;
    }
  }

  // Automatically calculated from unittests
  known_distances[IX('A', 4)][IX('B', 16)] = 440;
  known_distances[IX('B', 16)][IX('C', 10)] = 375;
  known_distances[IX('C', 10)][IX('B', 1)] = 371;
  known_distances[IX('B', 1)][IX('D', 14)] = 398;
  known_distances[IX('D', 14)][IX('E', 14)] = 287;
  known_distances[IX('E', 14)][IX('E', 9)] = 375;
  known_distances[IX('E', 9)][IX('D', 5)] = 621;
  known_distances[IX('D', 5)][IX('E', 6)] = 375;
  known_distances[IX('E', 6)][IX('D', 4)] = 297;
  known_distances[IX('D', 4)][IX('B', 6)] = 405;
  known_distances[IX('B', 6)][IX('C', 12)] = 354;
  known_distances[IX('C', 12)][IX('A', 4)] = 376;

  start_sensor_server();
  start_distance_server();
  start_location_server();

  // Setup Inner Loop for Track A
  init_trains();
  tr_sw(15, 'C');
  tr_sw(16, 'S');
  tr_sw(9, 'C');
  tr_sw(10, 'S');

  // Run the train
  int our_train = 47;
  int our_speed = 11;

  tr_set_speed(our_speed, our_train);
  printf(COM2, "Reverse train direction? (Y/N): ");
  char ch = Getc(COM2);
  printf(COM2, "%c\n", ch);
  if (ch == 'Y' || ch == 'y') {
    tr_reverse(our_train);
  }

  // Parsing Sensor Data
  sensor sensors[MAX_NEW_SENSORS];
  sensor last_sensor = (sensor) { 'X', 0 };

  while (1) {
    int num_sensors = get_sensor_data(sensors, MAX_NEW_SENSORS);
    for (i = 0; i < num_sensors; i++) {
      // Denoise sensor data
      if (sensors[i].group == last_sensor.group && sensors[i].socket == last_sensor.socket) {
        continue;
      }

      printf(COM2, "%c%d\n", sensors[i].group, sensors[i].socket);
      if (last_sensor.group != 'X') {
        int this_idx = IX(sensors[i].group, sensors[i].socket);
        int last_idx = IX(last_sensor.group, last_sensor.socket);

        if (known_distances[last_idx][this_idx] != UNKNOWN_DISTANCE) {
          last_seen[this_idx] = Time();
          int ts = times_seen[last_idx][this_idx] + 1;
          times_seen[last_idx][this_idx] = ts;

          if (ts == 1) {
            // Do nothing, we want to avoid accelerating measurements
          } else if (ts <= NUM_SAMPLES + 1) {
            recorded_times[last_idx][this_idx][ts - 2] = last_seen[this_idx] - last_seen[last_idx];
            if (ts == NUM_SAMPLES + 1) {
              int avg = average_recorded(last_idx, this_idx);
              int var = variance(last_idx, this_idx, avg);
              printf(COM2, "%s to %s: %d ms (variance. %d ms) ", track[last_idx].name, track[this_idx].name, avg, var);
              printf(COM2, "%d mm/s\n", (known_distances[last_idx][this_idx] * 1000) / avg);
            }
          }
        }
      }

      last_sensor = sensors[i];
    }
  }
}

void acceleration_calibration() {
  track_node* track = get_track();
  init_tracka(track);

  // Run the train
  int our_train = 47;
  int our_speed = 11;
  int i, a;

  sensor sensors[MAX_NEW_SENSORS];
  start_sensor_server();
  start_distance_server();
  start_location_server();
  init_trains();

  tr_set_speed(2, our_train);
  printf(COM2, "Reverse train direction? (Y/N): ");
  char ch = Getc(COM2);
  printf(COM2, "%c\n", ch);

  tr_set_speed(0, our_train);
  if (ch == 'Y' || ch == 'y') {
    tr_reverse(our_train);
  }

  sensor target_sensor = (sensor) { 'E', 14 };

  for (;;) {
    printf(COM2, "Put train Xcm from E14, then press any key: ");
    ch = Getc(COM2);
    printf(COM2, "%c\n", ch);

    int start = Time();
    tr_set_speed(our_speed, our_train);
    int hit_target = 0;
    while (hit_target < 2) {
      int num_sensors = get_sensor_data(sensors, MAX_NEW_SENSORS);
      for (i = 0; i < num_sensors; i++) {
        if (sensors[i].group == target_sensor.group && sensors[i].socket == target_sensor.socket) {
          hit_target++;
          break;
        }
      }
    }

    int end = Time();
    printf(COM2, "(Xcm) tX: %d\n", end - start);
    tr_set_speed(0, our_train);
  }
}

void calibration_task() {
  printf(COM2, "Starting Calibration\n");
  printf(COM2, "Acceleration Calibration (A) or Velocity Calibration (V)?: ");
  char ch = Getc(COM2);
  printf(COM2, "%c\n", ch);

  if (ch == 'A') {
    acceleration_calibration();
  } else if (ch == 'V') {
    velocity_calibration();
  }

  Exit();
}
