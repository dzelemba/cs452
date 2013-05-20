#include "syscall.h"

static Request request;

__attribute__ ((noinline)) static Request* get_request() {
  asm("");
  return &request;
}

int Create(int priority, void (*code)) {
  request.syscall = 0;
  request.args[0] = priority;
  request.args[1] = (int)code;

  get_request(); // Put request into r0
  asm("swi");

  register int retval asm("r0");
  return retval;
}

int MyTid() {
  request.syscall = 1;

  get_request(); // Put request into r0
  asm("swi");

  register int retval asm("r0");
  return retval;
}

int MyParentTid() {
  request.syscall = 2;

  get_request(); // Put request into r0
  asm("swi");

  register int retval asm("r0");
  return retval;
}

void Pass() {
  request.syscall = 3;

  get_request(); // Put request into r0
  asm("swi");
}


void Exit() {
  request.syscall = 4;

  get_request(); // Put request into r0
  asm("swi");
}

