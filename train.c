#include "train.h"
#include "syscall.h"
#include "uart.h"
#include "ourio.h"
#include "debug.h"
#include "priorities.h"

static int train_speeds[NUM_TRAINS + 1];
static int switch_directions[NUM_SWITCHES + 1];
static int reverse_server_tid;

/*
 * Helpers
 */

void fill_set_speed(char*cmd, int speed, int train) {
  cmd[0] = speed;
  cmd[1] = train;
}

void tr_reverse_task() {
  int tid;
  int train_info[2];
  while (1) {
    Receive(&tid, (char *)train_info, 2*sizeof(int));
    Reply(tid, (void *)0, 0);

    int train = train_info[0];
    int speed = train_info[1];

    char cmd[4];
    fill_set_speed(cmd, 0, train);
    putbytes(COM1, cmd, 2);

    Delay(300);
    fill_set_speed(cmd, 15, train);
    fill_set_speed(cmd + 2, speed, train);
    putbytes(COM1, cmd, 4);
  }
  Exit();
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

  reverse_server_tid = Create(MED_PRI_K, &tr_reverse_task);
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

  int train_info[2];
  train_info[0] = train;
  train_info[1] = train_speeds[train];

  Send(reverse_server_tid, (char *)train_info, 2*sizeof(int), (void *)0, 0);
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
