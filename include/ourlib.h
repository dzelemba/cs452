#ifndef __STDLIB_H__
#define __STDLIB_H__

#ifndef UNIT
#define bool char
#define true 1
#define false 0
#endif

void init_stdlib();

int min(int a, int b);

int max(int a, int b);

int abs(int a);

int atoi(char* src);

#ifndef UNIT
int memcpy(char* destination, const char* source, int len);
#endif

char* kmalloc(int size);

int in_userspace();

#endif
