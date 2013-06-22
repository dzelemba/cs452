#include "syscall.h"
#include "nameserver.h"
#include "task.h"

static Request request;

int syscall(Request* req) {
  asm("swi");
  register int retval asm("r0");
  return retval;
}

int Create(int priority, void (*code)) {
  request.syscall = CALLID_CREATE;
  request.args[0] = priority;
  request.args[1] = (int)code;

  return syscall(&request);
}

int MyTid() {
  request.syscall = CALLID_MYTID;

  return syscall(&request);
}

int MyParentTid() {
  request.syscall = CALLID_MYPARENTTID;

  return syscall(&request);
}

void Pass() {
  request.syscall = CALLID_PASS;

  syscall(&request);
}


void Exit() {
  request.syscall = CALLID_EXIT;

  syscall(&request);
}

int Send(int tid, char *msg, int msglen, char *reply, int replylen) {
  if (tid < 0 || tid > MAX_TASKS) {
    return -1;
  }

  request.syscall = CALLID_SEND;
  request.args[0] = tid;
  request.args[1] = (int)msg;
  request.args[2] = msglen;
  request.args[3] = (int)reply;
  request.args[4] = replylen;

  return syscall(&request);
}

int Receive(int *tid, char *msg, int msglen) {
  request.syscall = CALLID_RECEIVE;
  request.args[0] = (int)tid;
  request.args[1] = (int)msg;
  request.args[2] = msglen;

  return syscall(&request);
}

int Reply(int tid, char *reply, int replylen) {
  if (tid < 0 || tid > MAX_TASKS) {
    return -1;
  }

  request.syscall = CALLID_REPLY;
  request.args[0] = tid;
  request.args[1] = (int)reply;
  request.args[2] = replylen;

  return syscall(&request);
}

int AwaitEvent(int eventid) {
  request.syscall = CALLID_AWAITEVENT;
  request.args[0] = eventid;

  return syscall(&request);
}
