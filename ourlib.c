#include "ourlib.h"
#include "debug.h"
#include "timings.h"

#define HEAP_SIZE 16 * 1024 * 1024

static char heap[HEAP_SIZE];
static char* free_ptr;

void init_stdlib() {
  free_ptr = heap;
}

int min(int a, int b) {
  return a < b ? a : b;
}

int max(int a, int b) {
  return a > b ? a : b;
}

int abs(int a) {
  return a < 0 ? -1 * a : a;
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

int memcpy_aligned(int* dest, const int* src, int len) {
  int i;
  for (i = 0; i < len; i++) {
    dest[i] = src[i];
  }

  return 0;
}

int memcpy_unaligned(char* dest, const char * src, int len) {
  int i;
  for (i = 0; i < len; i++) {
    dest[i] = src[i];
  }

  return 0;
}

int memcpy(char* dest, const char* src, int len) {
/*
  if (!in_userspace()) {
    start_timing(KERNEL_MEMCPY);
  }
*/
  int ret;
  if ((unsigned int)dest % sizeof(int) == 0 &&
      (unsigned int)src % sizeof(int) == 0 &&
      len % sizeof(int) == 0) {
    ret = memcpy_aligned((int *)dest, (const int*)src, len / sizeof(int));
  } else {
    ret = memcpy_unaligned(dest, src, len);
  }
/*
  if (!in_userspace()) {
    end_timing(KERNEL_MEMCPY);
  }
*/
  return ret;
}

char* kmalloc(int size) {
  ASSERT(free_ptr + size < heap + HEAP_SIZE, "kmalloc out of memory");

  char* ret = free_ptr;
  free_ptr += size;
  return ret;
}

#ifndef UNIT
int in_userspace() {
  asm("mrs r3, cpsr");
  register int cpsr asm("r3");
  return (cpsr & 0x1f) == 0x10;
}
#endif
