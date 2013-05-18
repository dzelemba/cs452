#ifndef __SYSCALL_H__
#define __SYSCALL_H__

typedef struct Request {
  int syscall;
  int args[5];
} Request;

int Create(int priority, void (*code));

int MyTid();

int MyParentTid();

void Pass();

void Exit();

#endif
