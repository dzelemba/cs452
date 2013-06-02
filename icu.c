#include "icu.h"

void clear_soft_int() {
  *(int *)(VIC1_BASE + SOFT_INT_CLEAR_OFFSET) = 1;
}
