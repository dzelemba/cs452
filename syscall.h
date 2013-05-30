#ifndef __SYSCALL_H__
#define __SYSCALL_H__

#define CALLID_CREATE       0
#define CALLID_MYTID        1
#define CALLID_MYPARENTTID  2
#define CALLID_PASS         3
#define CALLID_EXIT         4
#define CALLID_SEND         5
#define CALLID_RECEIVE      6
#define CALLID_REPLY        7

// TODO(dzelemba): Fix where priority definitions are.
#include "scheduler.h"

typedef struct Request {
  int syscall;
  int args[5];
} Request;

int Create(int priority, void (*code));

int MyTid();

int MyParentTid();

void Pass();

void Exit();

int Send(int tid, char *msg, int msglen, char *reply, int replylen);

int Receive(int *tid, char *msg, int msglen);

int Reply(int tid, char *reply, int replylen);

/* Implemented in nameserver.c */

int RegisterAs(char* name);

int WhoIs(char *name);

#endif
