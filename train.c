#include "train.h"
#include "syscall.h"
#include "uart.h"
#include "stdio.h"
#include "debug.h"

static int train_speeds[NUM_TRAINS + 1];
static int switch_directions[NUM_SWITCHES + 1];

/*
 * Helpers
 */

void fill_set_speed(char*cmd, int speed, int train) {
  cmd[0] = speed;
  cmd[1] = train;
}

/*
 * Public Methods
 */

void init_trains() {
  int i;
  for (i = 1; i < NUM_TRAINS + 1; i++) {
    train_speeds[i] = 0;
  }

  for (i = 1; i < NUM_SWITCHES + 1; i++) {
    switch_directions[i] = '?';
  }

  /* Required settings */
  ua_setspeed(COM1, 2400);
  ua_setstopbits(COM1, ON);
  ua_setfifo(COM1, OFF);
}

/* Train Methods */

void tr_set_speed(int speed, int train) {
  ASSERT(train > 0 && train <= NUM_TRAINS, "train.c: tr_set_speed: Invalid train number");
  ASSERT(speed >= 0 && speed <= NUM_SPEEDS, "train.c: tr_set_speed: Invalid speed");

  char cmd[2];
  fill_set_speed(cmd, speed, train);

  putbytes(COM1, cmd, 2);
  train_speeds[train] = speed;
}

void tr_reverse(int train) {
  ASSERT(train > 0 && train <= NUM_TRAINS, "train.c: tr_reverse: Invalid train number");

  char cmd[6];
  fill_set_speed(cmd, 0, train);
  fill_set_speed(cmd + 2, 15, train);
  fill_set_speed(cmd + 4, train_speeds[train], train);

  putbytes(COM1, cmd, 6);
}

/* Switch Methods */

int convert_switch_number(int switch_number) {
  if (switch_number < 19) return switch_number;

  switch (switch_number) {
  case 0x99:
    return 19;
  case 0x9A:
    return 20;
  case 0x9B:
    return 21;
  case 0x9C:
    return 22;
  }

  return 0;
}

void tr_sw(int switch_number, char switch_direction) {
  int switch_direction_code;
  switch (switch_direction) {
  case 'S':
    switch_direction_code = 33;
    break;
  case 'C':
    switch_direction_code = 34;
    break;
  default:
    ERROR("train.c: tr_sw: Invalid switch direction");
    return;
  }

  char cmd[3];
  cmd[0] = switch_direction_code;
  cmd[1] = switch_number;
  cmd[2] = 32; /* Turn solenoid off */
  putbytes(COM1, cmd, 3);

  switch_directions[convert_switch_number(switch_number)] = switch_direction;
}

char get_switch_direction(int switch_number) {
  return switch_directions[switch_number];
}
