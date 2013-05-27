#include "stdlib.h"

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
