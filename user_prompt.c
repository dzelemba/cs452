#include "location_server.h"
#include "ourio.h"
#include "ourlib.h"
#include "physics.h"
#include "priorities.h"
#include "queue.h"
#include "sensor.h"
#include "sensor_server.h"
#include "strings.h"
#include "syscall.h"
#include "track_data.h"
#include "train.h"
#include "user_prompt.h"

#define MAX_LINE_LENGTH 64
#define MAX_TOKENS 4
#define MAX_TOKEN_SIZE 8

#define DRAW_ROW_RECENT_HIT 9
#define DRAW_ROW_TRAIN_LOC 16
#define DRAW_ROW_PROMPT 36
#define DRAW_DEBUG_OUTPUT 38
#define DEBUG_OUTPUT_SIZE 10

#define DRAW_ROW_LOG 26
#define LOG_LENGTH 10
static char log_mem[LOG_LENGTH][80];
static int log_ring;

#define DRAW_COL_TR_LOC 12
#define DRAW_COL_TR_DIST 23
#define DRAW_COL_TR_DIR 35
#define DRAW_COL_TR_ERR 56

static unsigned int current_prompt_pos;

void draw_switch_state(int switch_num, char direction) {
  int row, col;
  if (switch_num >= 0x99) {
    row = 7;
    col = (switch_num - 0x99) * 4 + 2;
  } else {
    row = 5;
    col = (switch_num - 1) * 4 + 2;
  }
  printf(COM2, "\033[%d;%dH%c", row, col, direction);
}

void return_cursor() {
  printf(COM2, "\033[%d;%uH", DRAW_ROW_PROMPT, current_prompt_pos + 2);
}

void draw_initial() {
  printf(COM2, "\033[2J");
  printf(COM2, "\033[3;1HSwitch Table:");
  printf(COM2, "\033[%d;1HMost Recently Hit Sensors:", DRAW_ROW_RECENT_HIT);
  printf(COM2, "\033[%d;1HTrain Locations:", DRAW_ROW_TRAIN_LOC);
  printf(COM2, "\033[%d;1HDebug Output:", DRAW_DEBUG_OUTPUT);

  // Set scrollable area for debug output.
  printf(COM2, "\033[%d;%dr", DRAW_DEBUG_OUTPUT + 1, DRAW_DEBUG_OUTPUT + DEBUG_OUTPUT_SIZE);

  // Draw switch states
  int sw;

  printf(COM2, "\033[4;1H");
  for (sw = 1; sw <= 18; sw++) {
    printf(COM2, "%3d ", sw);
  }
  for (sw = 1; sw <= 18; sw++) {
    draw_switch_state(sw, 'S');
  }

  printf(COM2, "\033[6;1H");
  for (sw = 0x99; sw <= 0x9c; sw++) {
    printf(COM2, "%3d ", sw);
  }

  for (sw = 0x99; sw <= 0x9c; sw++) {
    draw_switch_state(sw, 'S');
  }

  // Draw train states
  int tr;
  for (tr = 0; tr < MAX_TRAINS; tr++) {
    printf(COM2, "\033[%d;1HTr %d Loc: ____ Dist: ____mm Dir: _ Prev Sensor Error: _____mm\033[K\n",
           DRAW_ROW_TRAIN_LOC + tr + 1, tr_idx_to_num(tr));
  }

  return_cursor();
}

int check_train_number(int train) {
  return train >=1 && train <= NUM_TRAINS;
}

int check_train_speed(int speed) {
  return speed >= 0 && speed <= NUM_SPEEDS;
}

int process_line(char* line) {
  char static_tokens[MAX_TOKENS][MAX_TOKEN_SIZE];
  char* tokens[MAX_TOKENS];
  create_string_array((const char*)static_tokens, MAX_TOKENS, MAX_TOKEN_SIZE, (const char**)tokens);

  char* error = 0;
  int ret = string_split(line, ' ', MAX_TOKENS, MAX_TOKEN_SIZE, (char**)tokens, &error);
  if (ret == -1) {
    return 1;
  }

  if (string_equal(tokens[0], "tr")) {
    if (ret != 3) {
      return 1;
    }

    int train_number = atoi(tokens[1]);
    int train_speed = atoi(tokens[2]);
    if (!check_train_number(train_number) || !check_train_speed(train_speed)) {
      return 1;
    }

    tr_set_speed(train_speed, train_number);
  } else if (string_equal(tokens[0], "sw")) {
    if (ret != 3) {
      return 1;
    }

    int switch_number = atoi(tokens[1]);
    char direction = tokens[2][0];
    if (direction != 'C' && direction != 'S') {
      return 1;
    }

    if (!((switch_number >= 1 && switch_number <= 18) || (switch_number >= 153 && switch_number <= 156))) {
      return 1;
    }
    tr_sw(switch_number, direction);
    draw_switch_state(switch_number, direction);
  } else if (string_equal(tokens[0], "rv")) {
    if (ret != 2) {
      return 1;
    }

    int train_number = atoi(tokens[1]);
    if (!check_train_number(train_number)) {
      return 1;
    }
    tr_reverse(train_number);
  } else if (string_equal(tokens[0], "q")) {
    Shutdown();
  } else if (string_equal(tokens[0], "init")) {
    if (ret != 2) {
      return 1;
    }

    if (tokens[1][0] == 'A') {
      init_tracka(get_track());
      init_physicsa();
    } else if (tokens[1][0] == 'B') {
      init_trackb(get_track());
      init_physicsb();
    } else {
      return 1;
    }
  } else if (string_equal(tokens[0], "track")) {
    int train = atoi(tokens[1]);
    if (!check_train_number(train)) {
      return 1;
    }
    tr_track(train);
  } else if (string_equal(tokens[0], "goto")) {
    if (ret != 4) {
      return 1;
    }

    int train = atoi(tokens[1]);
    int speed = atoi(tokens[3]);
    if (!check_train_number(train) || !check_train_speed(speed)) {
      return 1;
    }

    location loc;
    init_location(&loc);
    if (tokens[2][0] == 'B') {
      loc.node = get_track_node(get_track(), branch2idx(atoi(&tokens[2][1])));
    } else if (tokens[2][0] == 'M') {
      loc.node = get_track_node(get_track(), merge2idx(atoi(&tokens[2][1])));
    } else if (tokens[2][0] == 'S') {
      loc.node = get_track_node(get_track(), sensor2idx(tokens[2][1], atoi(&tokens[2][2])));
    } else if (tokens[2][0] == 'E' && tokens[2][1] == 'N') {
      loc.node = get_track_node(get_track(), enter2idx(atoi(&tokens[2][2])));
    } else if (tokens[2][0] == 'E' && tokens[2][1] == 'X') {
      loc.node = get_track_node(get_track(), exit2idx(atoi(&tokens[2][2])));
    } else {
      return 1;
    }
    tr_set_route(train, speed, &loc);
  } else if (string_equal(tokens[0], "set_dir")) {
    if (ret != 3) {
      return 1;
    }

    int train = atoi(tokens[1]);
    if (!check_train_number(train)) {
      return 1;
    }

    if (tokens[2][0] == 'F') {
      ls_set_direction(train, FORWARD);
    } else if (tokens[2][0] == 'B') {
      ls_set_direction(train, BACKWARD);
    } else {
      return 1;
    }
  } else {
    return 1;
  }

  return 0;
}

void user_prompt_task() {
  char line[64];

  // Initialize log
  log_ring = 0;
  int i, j;
  for (i = 0; i < LOG_LENGTH; i++) {
    for (j = 0; j < 80; j++) {
      log_mem[i][j] = '\0';
    }
  }

  printf(COM2, "\033[%d;1H>\033[K", DRAW_ROW_PROMPT);

  while (1) {
    char ch = Getc(COM2);
    if (ch == '\r') {
      line[current_prompt_pos] = '\0'; // null-terminate line
      memcpy(log_mem[log_ring] + 5, line, current_prompt_pos);
      log_mem[log_ring][current_prompt_pos + 5 + 0] = '\033';
      log_mem[log_ring][current_prompt_pos + 5 + 1] = '[';
      log_mem[log_ring][current_prompt_pos + 5 + 2] = '0';
      log_mem[log_ring][current_prompt_pos + 5 + 3] = 'm';
      log_mem[log_ring][current_prompt_pos + 5 + 4] = '\0';

      int is_bad_command = process_line(line);
      if (is_bad_command) {
        memcpy(log_mem[log_ring], "\033[31m", 5);
      } else {
        memcpy(log_mem[log_ring], "\033[32m", 5);
      }

      // Update log
      for (i = 0; i < LOG_LENGTH; i++) {
        printf(COM2, "\033[%d;1H%s\033[K", DRAW_ROW_LOG + i, log_mem[(log_ring + i + 1) % LOG_LENGTH]);
      }
      log_ring = (log_ring + 1) % LOG_LENGTH;

      current_prompt_pos = 0;
      printf(COM2, "\033[%d;%uH\033[K", DRAW_ROW_PROMPT, current_prompt_pos + 2);
    } else if (ch == 8) { // Backspace
      if (current_prompt_pos > 0) {
        current_prompt_pos--;
        printf(COM2, "\033[%d;%uH\033[K", DRAW_ROW_PROMPT, current_prompt_pos + 2);
      }
    } else {
      if (current_prompt_pos < MAX_LINE_LENGTH - 1) {
        line[current_prompt_pos] = ch;
        current_prompt_pos++;
        printf(COM2, "\033[%d;%uH%c", DRAW_ROW_PROMPT, current_prompt_pos + 1, ch);
      }
    }
  }

  Exit();
}

/*
 * Displaying Sensor Data
 */

#define HITS_TRACKED 5
static sensor _last_seen_hits[HITS_TRACKED];

void _add_hit(char group, int socket) {
  int last_tracked = HITS_TRACKED - 1;
  int i;
  for (i = 0; i < HITS_TRACKED; i++) {
    if (_last_seen_hits[i].group == group && _last_seen_hits[i].socket == socket) {
      last_tracked = i;
      break;
    }
  }

  // shift hits back
  for (i = last_tracked; i >= 1; i--) {
    _last_seen_hits[i] = _last_seen_hits[i - 1];
  }
  _last_seen_hits[0] = (sensor){ group, socket };
}

void display_sensor_data() {
  int i;
  for (i = 0; i < HITS_TRACKED; i++) {
    _last_seen_hits[i] = (sensor){ '0', -1 };
  }

  sensor sensors[MAX_NEW_SENSORS];
  while (1) {
    int num_sensors = get_sensor_data(sensors, MAX_NEW_SENSORS);

    for (i = 0; i < num_sensors; i++) {
      _add_hit(sensors[i].group, sensors[i].socket);
    }
    for (i = 0; i < HITS_TRACKED && _last_seen_hits[i].group != '0'; i++) {
      printf(COM2, "\033[%d;1H%c%d\033[K\n", DRAW_ROW_RECENT_HIT + 1 + i, _last_seen_hits[i].group, _last_seen_hits[i].socket);
    }
    return_cursor();
  }
}

// TIMER TASK

void timer_display_task() {
  int ticks_per_tenth = 10;
  int ticks_per_second = 10 * 10;
  int ticks_per_minute = 10 * 10 * 60;

    // Block until screen is initialized

  while (1) {
    int ticks = Time();
    int display_minutes = ticks / ticks_per_minute;
    ticks = ticks % ticks_per_minute;
    int display_seconds = ticks / ticks_per_second;
    ticks = ticks % ticks_per_second;
    int display_tenths = ticks / ticks_per_tenth;

    printf(COM2, "\033[1;1H%d:%2d.%u", display_minutes, display_seconds, display_tenths);
    return_cursor();
    Delay(ticks_per_tenth);
  }

  Exit();
}

/*
 * Displaying Train Locations
 */

int has_location_changed_enough(location_array* cur_locs, location_array* prev_locs, int i) {
  if (prev_locs->size == 0) {
    return 1;
  }
  location* cur_loc = &cur_locs->locations[i];
  location* prev_loc = &prev_locs->locations[i];

  return cur_loc->node != prev_loc->node ||
         cur_loc->d != prev_loc->d ||
         cur_loc->um_past_node - prev_loc->um_past_node > 100000;
}

void display_train_locations() {
  location_array prev_locations;
  location_array loc_array;
  int i;
  while (1) {
    get_location_updates(&loc_array);
    for (i = 0; i < loc_array.size; i++) {
      if (has_location_changed_enough(&loc_array, &prev_locations, i)) {
        location* loc = &loc_array.locations[i];
        printf(COM2, "\033[33m\033[%d;%dH%4s\033[%dC%4d\033[%dC%s\033[%dC%5d\033[0m",
               DRAW_ROW_TRAIN_LOC + 1 + tr_num_to_idx(loc->train), DRAW_COL_TR_LOC, loc->node->name,
               DRAW_COL_TR_DIST - DRAW_COL_TR_LOC - 4, loc->um_past_node / 1000,
               DRAW_COL_TR_DIR - DRAW_COL_TR_DIST - 4, direction_to_string(loc->d),
               DRAW_COL_TR_ERR - DRAW_COL_TR_DIR - 1, loc->prev_sensor_error);
        return_cursor();
      }
    }

    memcpy((char *)&prev_locations, (const char*)&loc_array, sizeof(location_array));
  }

  Exit();
}

/*
 * Public Methods
 */

void print_debug_output(char* fmt, ...) {
  va_list va;
  va_start(va,fmt);
  format_debug_output(fmt, va);
  va_end(va);

}

void format_debug_output(char* fmt, va_list va) {
  string output;
  STR_CREATE(output, MAX_DEBUG_LENGTH);

  s_format(&output, fmt, va);

  printf(COM2, "\033[%d;1H\n%s", DRAW_DEBUG_OUTPUT + DEBUG_OUTPUT_SIZE, str_get_chars(&output));
}

void start_user_prompt() {
  current_prompt_pos = 0;
  draw_initial();
  Create(MED_PRI, &timer_display_task);
  Create(MED_PRI, &user_prompt_task);
  Create(MED_PRI, &display_sensor_data);
  Create(MED_PRI, &display_train_locations);
}
