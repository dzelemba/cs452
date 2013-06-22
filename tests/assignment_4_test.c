#include "all_tests.h"
#include "kernel.h"
#include "priorities.h"
#include "stdio.h"
#include "stdlib.h"
#include "strings.h"
#include "syscall.h"
#include "task.h"
#include "test_helpers.h"
#include "train.h"

// DRAWING LIBRARY
static unsigned int current_prompt_pos;
static int shutdown;

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

void draw_initial() {
  printf(COM2, "\033[2J\033[1;1H");
  printf(COM2, "\033[2;1HBUSY\r");
  printf(COM2, "Switch Table:");
  printf(COM2, "\033[13;1HMost Recently Hit Sensors:");

  int sw;

  // top row
  printf(COM2, "\033[4;1H");
  for (sw = 1; sw <= 18; sw++) {
    printf(COM2, "%3d ", sw);
  }

  for (sw = 1; sw <= 18; sw++) {
    draw_switch_state(sw, 'S');
  }

  // bottom row
  printf(COM2, "\033[6;1H");
  for (sw = 0x99; sw <= 0x9c; sw++) {
    printf(COM2, "%3d ", sw);
  }

  for (sw = 0x99; sw <= 0x9c; sw++) {
    draw_switch_state(sw, 'S');
  }
}

// USER PROMPT TASK
// Maintain invariant that before every printf, we are at the cursor prompt pos

#define MAX_LINE_LENGTH 64

void user_prompt_task() {
  current_prompt_pos = 0;
  char line[64];

  char static_tokens[3][8];
  char* tokens[3];
  create_string_array((const char*)static_tokens, 3, 8, (const char**)tokens);

  printf(COM2, "\033[2;1H>\033[K");
  while (1) {
    char ch = Getc(COM2);
    if (ch == '\r') {
      line[current_prompt_pos] = '\0'; // null-terminate line

      char* error = 0;
      int ret = string_split(line, ' ', 3, 8, (char**)tokens, &error);
      if (ret != -1) {
        if (string_equal(tokens[0], "tr")) {
          int train_number = atoi(tokens[1]);
          int train_speed = atoi(tokens[2]);
          tr_set_speed(train_speed, train_number);
        } else if (string_equal(tokens[0], "sw")) {
          int switch_number = atoi(tokens[1]);
          char direction = tokens[2][0];
          tr_sw(switch_number, direction);
          draw_switch_state(switch_number, direction);
        } else if (string_equal(tokens[0], "rv")) {
          int train_number = atoi(tokens[1]);
          tr_reverse(train_number);
        } else if (string_equal(tokens[0], "q")) {
          shutdown = 1;
          break;
        } else {
          // bad command
        }
      }

      current_prompt_pos = 0;
      printf(COM2, "\033[2;%uH\033[K", current_prompt_pos + 2);
    } else if (ch == 8) { // Backspace
      if (current_prompt_pos > 0) {
        current_prompt_pos--;
        printf(COM2, "\033[2;%uH\033[K", current_prompt_pos + 2);
      }
    } else {
      if (current_prompt_pos < MAX_LINE_LENGTH - 1) {
        line[current_prompt_pos] = ch;
        current_prompt_pos++;
        printf(COM2, "\033[2;%uH%c", current_prompt_pos + 1, ch);
      }
    }
  }

  Exit();
}

// TIMER TASK

void timer_display_task() {
  int ticks_per_tenth = 10;
  int ticks_per_second = 10 * 10;
  int ticks_per_minute = 10 * 10 * 60;

  // Block until screen is initialized
  int shell_tid;
  Receive(&shell_tid, (char *)0, 0);
  Reply(shell_tid, (char *)0, 0);

  while (1) {
    if (shutdown) {
      break;
    }

    int ticks = Time();
    int display_minutes = ticks / ticks_per_minute;
    ticks = ticks % ticks_per_minute;
    int display_seconds = ticks / ticks_per_second;
    ticks = ticks % ticks_per_second;
    int display_tenths = ticks / ticks_per_tenth;

    printf(COM2, "\033[1;1H%d:%2d.%u", display_minutes, display_seconds, display_tenths);
    Delay(ticks_per_tenth);
  }

  Exit();
}

// SENSOR TASK

typedef struct sock {
  char sensor;
  int socket;
} sock;

#define HITS_TRACKED 5
static sock _last_seen_hits[HITS_TRACKED];

void _add_hit(char sensor, int socket) {
  int last_tracked = HITS_TRACKED - 1;
  int i;
  for (i = 0; i < HITS_TRACKED; i++) {
    if (_last_seen_hits[i].sensor == sensor && _last_seen_hits[i].socket == socket) {
      last_tracked = i;
      break;
    }
  }

  // shift hits back
  for (i = last_tracked; i >= 1; i--) {
    _last_seen_hits[i] = _last_seen_hits[i - 1];
  }
  _last_seen_hits[0] = (sock){ sensor, socket };
}

void sensor_task() {
  int i;
  for (i = 0; i < HITS_TRACKED; i++) {
    _last_seen_hits[i] = (sock){ '0', -1 };
  }

  while (1) {
    if (shutdown) {
      break;
    }

    update_sensors();
    char sensor;
    int socket;
    for (sensor = 'A'; sensor <= 'E'; sensor++) {
      for (socket = 1; socket <= SOCKETS_PER_SENSOR; socket++) {
        int hit = get_sensor_data(sensor, socket);
        if (hit) {
          _add_hit(sensor, socket);
        }
      }
    }

    for (i = 0; i < HITS_TRACKED && _last_seen_hits[i].sensor != '0'; i++) {
      printf(COM2, "\033[%d;1H%c%d\033[K\n", 14 + i, _last_seen_hits[i].sensor, _last_seen_hits[i].socket);
    }
  }

  Exit();
}

// SHELL TASK

void shell_task() {
  shutdown = 0;

  draw_initial();

  int timer_display_tid = Create(MED_PRI, &timer_display_task);
  Send(timer_display_tid, (char*)0, 0, (char *)0, 0); // No longer necessary because of priority hacks
  int user_prompt_tid = Create(MED_PRI, &user_prompt_task);
  int sensor_tid = Create(MED_PRI, &sensor_task);

  Exit();
}

void run_assignment_4_test() {
  char* name = "Assignment 4 Test";
  start_test(name);

  kernel_add_task(VLOW_PRI, &shell_task);
  kernel_run();
  end_test(name);
}
