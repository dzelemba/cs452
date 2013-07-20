#include "debug.h"
#include "ourio.h"
#include "priorities.h"
#include "switch_server.h"
#include "syscall.h"
#include "track_data.h"
#include "track_edge_array.h"
#include "user_prompt.h"

static int switch_directions[NUM_SWITCHES + 1];

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

int switch_number_from_index(int index) {
  if (index < 19) return index;

  switch (index) {
  case 19:
    return 0x99;
  case 20:
    return 0x9A;
  case 21:
    return 0x9B;
  case 22:
    return 0x9C;
  }

  return 0;
}

void sw(int switch_number, char switch_direction) {
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

  track_node* node = get_track_node(branch2idx(switch_number));
  track_edge* edge = &node->edge[(switch_direction == 'S') ? DIR_STRAIGHT : DIR_CURVED];
  if (isset_edge(get_broken_edges(), edge)) {
    return;
  }

  char cmd[3];
  cmd[0] = switch_direction_code;
  cmd[1] = switch_number;
  cmd[2] = 32; /* Turn solenoid off */
  putbytes(COM1, cmd, 3);

  switch_directions[convert_switch_number(switch_number)] = switch_direction;
  draw_switch_state(switch_number, switch_direction);
}

/*
 * Switch Server
 */

static int switch_server_tid;

typedef enum switch_server_message_type {
  SS_CHANGE_SWITCH,
  SS_GET_SWITCH_STATUS
} switch_server_message_type;

typedef struct switch_server_message {
  switch_server_message_type type;
  int switch_number;
  char switch_direction;
} switch_server_message;

void switch_server() {
  int tid;
  char direction;
  switch_server_message msg;
  while (1) {
    Receive(&tid, (char *)&msg, sizeof(switch_server_message));
    switch (msg.type) {
      case SS_CHANGE_SWITCH:
        Reply(tid, (char *)0, 0);
        sw(msg.switch_number, msg.switch_direction);
        break;
      case SS_GET_SWITCH_STATUS:
        direction = switch_directions[convert_switch_number(msg.switch_number)];
        Reply(tid, (char *)&direction, sizeof(char));
        break;
    }
  }
}


/*
 * Public Methods
 */

void init_switch_server() {
  int i;
  for (i = 1; i <= NUM_SWITCHES; i++) {
    sw(switch_number_from_index(i), 'S');
  }

  // Switch middle guys to curved since we'll never go straight there.
  sw(153, 'C');
  sw(154, 'C');
  sw(155, 'C');
  sw(156, 'C');

  switch_server_tid = Create(MED_PRI, &switch_server);
}

void tr_sw(int switch_number, char switch_direction) {
  switch_server_message msg;
  msg.type = SS_CHANGE_SWITCH;
  msg.switch_number = switch_number;
  msg.switch_direction = switch_direction;
  Send(switch_server_tid, (char *)&msg, sizeof(switch_server_message), (char *)0, 0);
}

char get_switch_direction(int switch_number) {
  switch_server_message msg;
  msg.type = SS_GET_SWITCH_STATUS;
  msg.switch_number = switch_number;
  char ret;
  Send(switch_server_tid, (char *)&msg, sizeof(switch_server_message), (char *)&ret, sizeof(char));

  return ret;
}
