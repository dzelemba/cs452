#ifndef __USER_PROMPT_H__
#define __USER_PROMPT_H__

#define MAX_DEBUG_LENGTH 128

#include "location.h"
#include "ourio.h"

void start_user_prompt();

void print_debug_output(char* fmt, ...);

void format_debug_output(char* fmt, va_list va);

void return_cursor();

void init_demo_output(int train1, int train2);

void update_demo_location(int row, location* loc);

void draw_switch_state(int switch_num, char direction);

#endif
