#include "linked_array.h"
#include "ourlib.h"
#include "debug.h"

struct node {
  void* data;

  struct node* next;
  struct node* prev;
  int position;

  int valid;
};

/* Private Methods */

static node* get_node(linked_array* la, int i) {
  return la->nodes + i;
}

static void clear_node(node* n) {
  n->data = 0;
  n->next = 0;
  n->prev = 0;
  n->valid = 0;
}

/* Public Methods */

void la_create(linked_array* la, int size) {
  la->size = size;

  la->head = 0;
  la->nodes = (node* )kmalloc(size * sizeof(node));

  int i;
  for (i = 0; i < size; i++) {
    node* n = get_node(la, i);
    clear_node(n);
    n->position = i;
  }
}

void la_insert(linked_array* la, int pos, void* data) {
  node* n = get_node(la, pos);
  ASSERT(!n->valid, "la_insert");

  n->data = data;
  n->valid = 1;

  if (la_is_empty(la)) {
    la->head = n;
    n->next = 0;
    n->prev= 0;
    return;
  }

  // Otherwise we do a linear scan to see where to insert the node.
  node* prev = 0;
  node* it = la->head;
  while (it != 0) {
    if (it->position > pos) {
      break;
    }
    prev = it;
    it = it->next;
  }

  // Now prev holds the node we must insert after.
  if (prev == 0) {
    n->next = la->head;
    n->prev = 0;
    la->head->prev = n;
    la->head = n;
  } else {
    n->next = prev->next;
    n->prev = prev;

    if (prev->next) {
      prev->next->prev = n;
    }
    prev->next = n;
  }
}

int la_has_element(linked_array* la, int pos) {
  return get_node(la, pos)->valid;
}

void* la_get_element(linked_array* la, int pos) {
  return get_node(la, pos)->data;
}

void* la_head(linked_array* la) {
  ASSERT(!la_is_empty(la), "la_head");
  return la->head->data;
}

void la_remove(linked_array* la, int pos) {
  node* n = get_node(la, pos);

  ASSERT(n->valid, "la_remove");
  if (n->prev) {
    n->prev->next = n->next;
  }
  if (n->next) {
    n->next->prev = n->prev;
  }

  if (la->head == n) {
    la->head = n->next;
  }

  clear_node(n);
}

int la_is_empty(linked_array* la) {
  return la->head == 0;
}

/*
 * Itertators
 */

void la_it_create(linked_array* la, linked_array_iterator* la_it) {
  la_it->pos = la->head;
}

int la_it_has_next(linked_array* la, linked_array_iterator* la_it) {
  (void)la; // Ignore compiler warnings.

  return la_it->pos != 0;
}

void* la_it_get_next(linked_array* la, linked_array_iterator* la_it) {
  (void)la;

  void* retval = la_it->pos->data;
  la_it->pos = la_it->pos->next;
  return retval;
}
