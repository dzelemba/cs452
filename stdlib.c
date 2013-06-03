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
