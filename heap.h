#ifndef __HEAP_H__
#define __HEAP_H__

#include "task.h"

typedef struct _heap_node {
  int priority;
  void *value;
} heap_node;

typedef struct _heap {
  heap_node* buf;
  unsigned char size;
} heap;

void init_heap(heap* hp, heap_node* buf, int size);

void* heap_min_value(heap* hp);

int heap_min_pri(heap* hp);

void* heap_delete_min(heap* hp);

void heap_insert(heap* hp, int priority, void* value);

#endif
