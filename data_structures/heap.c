#include "debug.h"
#include "heap.h"

// TODO: Better SWAP

void init_heap(heap* hp, heap_node* buf, int size) {
  hp->size = 0;
  hp->buf = buf;
}

int heap_min_pri(heap* hp) {
  return hp->buf[0].priority;
}

void* heap_min_value(heap* hp) {
  return hp->buf[0].value;
}

void* heap_delete_min(heap* hp) {
  void* mn = heap_min_value(hp);
  hp->size = hp->size - 1;

  heap_node* buf = hp->buf;

  unsigned char it = 0;
  buf[0] = buf[hp->size];

  // bubble-down
  while (1) {
    unsigned char left_child = 2 * it + 1;
    unsigned char right_child = left_child + 1;

    if (left_child >= hp->size) {
      return mn;
    }

    if (buf[left_child].priority < buf[it].priority &&
        ((right_child >= hp->size) // no right-child
         || buf[left_child].priority <= buf[right_child].priority)) {
      heap_node swap = buf[it];
      buf[it] = buf[left_child];
      buf[left_child] = swap;

      it = left_child;
    } else if (right_child < hp->size && buf[it].priority > buf[right_child].priority) {
      heap_node swap = buf[it];
      buf[it] = buf[right_child];
      buf[right_child] = swap;

      it = right_child;
    } else {
      return mn;
    }
  }
}

void heap_insert(heap* hp, int priority, void* value) {
  heap_node* buf = hp->buf;
  unsigned char it = hp->size;

  buf[it].priority = priority;
  buf[it].value = value;
  hp->size = hp->size + 1;

  while (1) {
    if (it == 0) {
      return;
    }

    unsigned char parent = (it - 1) / 2;
    if (buf[it].priority < buf[parent].priority) {
      heap_node swap = buf[it];
      buf[it] = buf[parent];
      buf[parent] = swap;

      it = parent;
    } else {
      return;
    }
  }
}
