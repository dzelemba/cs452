#ifndef __HEAPPLUS_H__
#define __HEAPPLUS_H__

#include "task.h"
#include "heap.h"

typedef struct _heapplus {
  heap h;
  int* dict;
} heapplus;

// Worst API to date!
void init_heapplus(heapplus* hp, heap_node* buf, int* dict, int size);

int heapplus_min_value(heapplus* hp);

int heapplus_min_pri(heapplus* hp);

int heapplus_priority(heapplus* hp, int value);

int heapplus_delete_min(heapplus* hp);

// This is insert or decreasekey
int heapplus_insert(heapplus* hp, int priority, int value);

int heapplus_size(heapplus* hp);

#endif
