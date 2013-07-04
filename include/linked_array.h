#ifndef __LINKED_ARRAY_H__
#define __LINKED_ARRAY_H__

typedef struct node node;

typedef struct linked_array {
  int size;
  node* head;
  node* nodes;
} linked_array;

/*
 * Creates a linked array of size 'size'.
 */
void la_create(linked_array* la, int size);

/*
 * Inserts the data into position pos of the array.
 * O(n) time.
 */
void la_insert(linked_array* la, int pos, void* data);

/*
 * Returns true if the node in position i is valid.
 */
int la_has_element(linked_array* la, int pos);

/*
 * Returns the node in position 'pos'.
 */
void* la_get_element(linked_array* la, int pos);

/*
 * Returns the first non-empty item in the array
 * O(1) time.
 */
void* la_head(linked_array* la);

/*
 * Removes position 'pos' from the array.
 */
void la_remove(linked_array* la, int pos);

/*
 * Returns true if the given linked array is empty.
 */
int la_is_empty(linked_array* la);

// ITERATOR

typedef struct linked_array_iterator {
 node* pos;
} linked_array_iterator;

void la_it_create(linked_array* la, linked_array_iterator* la_it);

int la_it_has_next(linked_array* la, linked_array_iterator* la_it);

void* la_it_get_next(linked_array* la, linked_array_iterator* la_it);

#endif
