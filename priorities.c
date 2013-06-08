#include "priorities.h"

int is_kernel_priority(int priority) {
  return priority == MAX_PRI ||
        priority == HI_PRI_K ||
        priority == MED_PRI_K ||
        priority == LOW_PRI_K ||
        priority == VLOW_PRI_K ||
        priority == MIN_PRI;
}
