#include "syscall.h"

static Request request;

// TODO(f2fung): This isn't necessary. Something similar to get_request should
// be done ONCE, and then the kernel no longer needs it passed along. Perf++
__attribute__ ((noinline)) static Request* get_request() {
  asm("");
  return &request;
}

int Create(int priority, void (*code)) {
  request.syscall = CALLID_CREATE;
  request.args[0] = priority;
  request.args[1] = (int)code;

  get_request(); // Put request into r0
  asm("swi");

  register int retval asm("r0");
  return retval;
}

int MyTid() {
  request.syscall = CALLID_MYTID;

  get_request(); // Put request into r0
  asm("swi");

  register int retval asm("r0");
  return retval;
}

int MyParentTid() {
  request.syscall = CALLID_MYPARENTTID;

  get_request(); // Put request into r0
  asm("swi");

  register int retval asm("r0");
  return retval;
}

void Pass() {
  request.syscall = CALLID_PASS;

  get_request(); // Put request into r0
  asm("swi");
}


void Exit() {
  request.syscall = CALLID_EXIT;

  get_request(); // Put request into r0
  asm("swi");
}

int Send(int tid, char *msg, int msglen, char *reply, int replylen) {
  request.syscall = CALLID_SEND;
  request.args[0] = tid;
  request.args[1] = (int)msg;
  request.args[2] = msglen;
  request.args[3] = (int)reply;
  request.args[4] = replylen;

  get_request(); // Put request into r0
  asm("swi");

  register int retval asm("r0");
  return retval;
}

int Receive(int *tid, char *msg, int msglen) {
  request.syscall = CALLID_RECEIVE;
  request.args[0] = (int)tid;
  request.args[1] = (int)msg;
  request.args[2] = msglen;

  get_request(); // Put request into r0
  asm("swi");

  register int retval asm("r0");
  return retval;
}

int Reply(int tid, char *reply, int replylen) {
  request.syscall = CALLID_REPLY;
  request.args[0] = tid;
  request.args[1] = (int)reply;
  request.args[2] = replylen;

  get_request(); // Put request into r0
  asm("swi");

  register int retval asm("r0");
  return retval;
}
