#include "syscall.h"

static Request request;

Request* get_request() {
  return &request;
}
void pass() {
  asm("swi 2");
}

void create(int priority, void (*code)) {
  request.syscall = 42;
  request.args[0] = priority;
  request.args[1] = (int)code;

  get_request(); // Put request into r0
  asm("swi");
}

void Exit() {
  // Fill in args;

  asm("swi 3");
}

