#ifndef __USER_PROMPT_H__
#define __USER_PROMPT_H__

#define MAX_DEBUG_LENGTH 128

#include "ourio.h"

void start_user_prompt();

void print_debug_output(char* fmt, ...);

void format_debug_output(char* fmt, va_list va);

void return_cursor();

#endif
