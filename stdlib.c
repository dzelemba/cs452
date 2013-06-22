#include "stdlib.h"
#include "debug.h"

#define HEAP_SIZE 1024 * 1024

static char heap[HEAP_SIZE];
static char* free_ptr;

void init_stdlib() {
  free_ptr = heap;
}

int min(int a, int b) {
  return a < b ? a : b;
}

int atoi(char* src) {
  int num = 0;
  while ((*src) != 0) {
    num *= 10;
    num += ((*src) - '0');
    src++;
  }
  return num;
}

int memcpy(char* destination, const char* source, int len) {
  int i;
  for (i = 0; i < len; i++) {
    *(destination + i) = *(source + i);
  }

  return 0;
}

char* kmalloc(int size) {
  ASSERT(free_ptr + size < heap + HEAP_SIZE, "kmalloc out of memory");

  char* ret = free_ptr;
  free_ptr += size;
  return ret;
}

int in_userspace() {
  asm("mrs r3, cpsr");
  register int cpsr asm("r3");
  return (cpsr & 0x1f) == 0x10;
}
