#include "all_tests.h"
#include "kernel.h"
#include "priorities.h"
#include "syscall.h"
#include "task.h"
#include "test_helpers.h"

static int tasks_run;

static void simple_delay_test() {
  int delay_time = 42;

  int start_time = Time();
  Delay(delay_time);
  int end_time = Time();

  assert_true(start_time + delay_time <= end_time, "Expected end time to be 42 ticks more than start time");

  tasks_run++;
  Exit();
}

static void simple_delay_until_test() {
  int delay_time = 61;
  int start_time = Time();
  DelayUntil(start_time + delay_time);
  int end_time = Time();

  assert_true(start_time + delay_time <= end_time, "Expected end time to be 61 ticks more than start time");

  tasks_run++;
  Exit();
}

static void empty_delay_test() {
  int start_time = Time();
  Delay(0);
  int end_time = Time();

  assert_true(start_time == end_time, "Warning: Delay(0) took longer than 1 tick");
  assert_true(start_time + 10 >= end_time, "Error: Delay(0) took longer than 10 ticks");

  start_time = Time();
  Delay(-10);
  end_time = Time();

  assert_true(start_time == end_time, "Warning: Delay(-10) took longer than 1 tick");
  assert_true(start_time + 10 >= end_time, "Error: Delay(-10) took longer than 10 ticks");

  start_time = Time();
  DelayUntil(start_time);
  end_time = Time();

  assert_true(start_time == end_time, "Warning: DelayUntil(Now) took longer than 1 tick");
  assert_true(start_time + 10 >= end_time, "Error: DelayUntil(Now) took longer than 10 ticks");

  start_time = Time();
  DelayUntil(start_time - 300);
  end_time = Time();

  assert_true(start_time == end_time, "Warning: DelayUntil(Past) took longer than 1 tick");
  assert_true(start_time + 10 >= end_time, "Error: DelayUntil(Past) took longer than 10 ticks");

  tasks_run++;
  Exit();
}

static void first() {
  int tid;
  tid = Create(VLOW_PRI_1, &simple_delay_test);
  while (task_get_state(tid) != ZOMBIE) {
    Delay(10);
  }

  tid = Create(VLOW_PRI_1, &simple_delay_until_test);
  while (task_get_state(tid) != ZOMBIE) {
    Delay(10);
  }

  tid = Create(VLOW_PRI_1, &empty_delay_test);
  while (task_get_state(tid) != ZOMBIE) {
    Delay(10);
  }

  Exit();
}

void run_clockserver_test() {
  char* name = "Clockserver Test";
  start_test(name);
  tasks_run = 0;

  kernel_add_task(VLOW_PRI, &first);

  kernel_run();

  assert_int_equals(3, tasks_run, "Clockserver Test: Tasks Didn't Run");

  end_test(name);
}
