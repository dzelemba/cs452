#ifndef __SWITCH_SERVER_H__
#define __SWITCH_SERVER_H__

#define NUM_SWITCHES 22

void init_switch_server();

void tr_sw(int switch_number, char switch_direction);

char get_switch_direction(int switch_number);

#endif
