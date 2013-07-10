#ifndef __TIMINGS_H_
#define __TIMINGS_H_

#include "task.h"

#define KERNEL (MAX_TASKS + 1)

void init_timings();

void start_timing_task(Task* t);

void end_timing_task(Task* t);

void start_timing(int process);

void end_timing(int process);

void print_timings();

#endif

