#ifndef __STDLIB_H__
#define __STDLIB_H__

void init_stdlib();

int min(int a, int b);

int memcpy(char* destination, const char* source, int len);

char* kmalloc(int size);

int in_userspace();

#endif
