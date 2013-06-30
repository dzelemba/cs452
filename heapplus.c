#include "heap.h"
#include "heapplus.h"

#define NOT_USED -1

void init_heapplus(heapplus* hp, heap_node* buf, int* dict, int size) {
  init_heap(&hp->h, buf, size);
  hp->dict = dict;
  int i;
  for (i = 0; i < size; i++) {
    hp->dict[i] = NOT_USED;
  }
}

int heapplus_min_value(heapplus* hp) {
  return (int) heap_min_value(&hp->h);
}

int heapplus_min_pri(heapplus* hp) {
  return heap_min_pri(&hp->h);
}

int heapplus_delete_min(heapplus* hp) {
  heap* hp_simple = &(hp->h);
  int mn = (int) heap_min_value(hp_simple);
  hp->dict[(int) mn] = NOT_USED;
  hp_simple->size = hp_simple->size - 1;

  heap_node* buf = hp_simple->buf;

  unsigned char it = 0;
  if (hp_simple->size > 0) {
    buf[0] = buf[hp_simple->size];
    hp->dict[(int) buf[0].value] = 0;
  }

  // bubble-down
  while (1) {
    unsigned char left_child = 2 * it + 1;
    unsigned char right_child = left_child + 1;

    if (left_child >= hp_simple->size) {
      return mn;
    }

    if (buf[left_child].priority < buf[it].priority &&
        ((right_child >= hp_simple->size) // no right-child
         || buf[left_child].priority <= buf[right_child].priority)) {
      heap_node swap = buf[it];
      buf[it] = buf[left_child];
      buf[left_child] = swap;
      hp->dict[(int) buf[it].value] = it;
      hp->dict[(int) buf[left_child].value] = left_child;

      it = left_child;
    } else if (right_child < hp_simple->size && buf[it].priority > buf[right_child].priority) {
      heap_node swap = buf[it];
      buf[it] = buf[right_child];
      buf[right_child] = swap;
      hp->dict[(int) buf[it].value] = it;
      hp->dict[(int) buf[right_child].value] = right_child;

      it = right_child;
    } else {
      return mn;
    }
  }
}

// This is insert of decreasekey
int heapplus_insert(heapplus* hp, int priority, int value) {
  heap* hp_simple = &(hp->h);
  heap_node* buf = hp_simple->buf;

  unsigned char it;
  if (hp->dict[(int) value] == NOT_USED) {
    it = hp_simple->size;
    buf[it].priority = priority;
    buf[it].value = (void *)value;
    hp->dict[(int)value] = it;
    hp_simple->size = hp_simple->size + 1;
  } else if (hp->dict[(int) value] > priority) {
    it = hp->dict[(int) value];
  } else {
    return 0;
  }

  while (1) {
    if (it == 0) {
      return 1;
    }

    unsigned char parent = (it - 1) / 2;
    if (buf[it].priority < buf[parent].priority) {
      heap_node swap = buf[it];
      buf[it] = buf[parent];
      buf[parent] = swap;
      hp->dict[(int) buf[it].value] = it;
      hp->dict[(int) buf[parent].value] = parent;

      it = parent;
    } else {
      return 1;
    }
  }
}

int heapplus_size(heapplus* hp) {
  return (hp->h).size;
}
