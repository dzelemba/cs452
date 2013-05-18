#ifndef __SYSCALL_H__
#define __SYSCALL_H__

typedef struct Request {
  int syscall;
  int args[5];
} Request;

void pass();

void create(int priority, void (*code));

void Exit();

#endif
