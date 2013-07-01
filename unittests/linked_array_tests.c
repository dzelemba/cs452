#include "linked_array.h"
#include "test_helpers.h"
#include <stdio.h>

#define SIZE 10

void basic_test() {
  printf("Basic Test\n");

  int data[SIZE];
  int i;
  for (i = 0; i < SIZE; i++) {
    data[i] = i;
  }

  linked_array la;
  la_create(&la, SIZE);

  assert_true(la_is_empty(&la), "Basic Test Is Empty");

  for (i = 0; i < SIZE; i++) {
    assert_false(la_has_element(&la, i), "Basic Test Pre-Insertion");
    la_insert(&la, i, (void *)&data[i]);
    assert_true(la_has_element(&la, i), "Basic Test Post-Insertion");
    assert_int_equals(data[i], *(int *)la_get_element(&la, i), "Basic Test get_element");
  }

  assert_false(la_is_empty(&la), "Basic Test Is Not Empty");

  for (i = 0; i < SIZE; i++) {
    assert_true(la_has_element(&la, i), "Basic Test Pre-Deletion");
    assert_int_equals(data[i], *(int *)la_head(&la), "Basic Test Head Check");
    la_remove(&la, i);
    assert_false(la_has_element(&la, i), "Basic Test Post-Deletion");
  }

  assert_true(la_is_empty(&la), "Basic Test Is Empty Final");
}

void insertion_removal_test() {
  printf("Insertion/Removal Test\n");

  int data[SIZE];
  int i;
  for (i = 0; i < SIZE; i++) {
    data[i] = i;
  }

  linked_array la;
  la_create(&la, SIZE);

  int pos, head;

  pos = 5; head = 5;
  la_insert(&la, pos, (void *)&data[head]);
  assert_int_equals(data[head], *(int *)la_head(&la), "Insertion Test Head Check 1");

  pos = 3; head = 3;
  la_insert(&la, pos, (void *)&data[head]);
  assert_int_equals(data[head], *(int *)la_head(&la), "Insertion Test Head Check 2");

  pos = 4; head = 3;
  la_insert(&la, pos, (void *)&data[head]);
  assert_int_equals(data[head], *(int *)la_head(&la), "Insertion Test Head Check 3");

  pos = 8; head = 3;
  la_insert(&la, pos, (void *)&data[head]);
  assert_int_equals(data[head], *(int *)la_head(&la), "Insertion Test Head Check 4");

  pos = 8; head = 3;
  la_remove(&la, pos);
  assert_int_equals(data[head], *(int *)la_head(&la), "Insertion Test Head Check 4");

  pos = 4; head = 3;
  la_remove(&la, pos);
  assert_int_equals(data[head], *(int *)la_head(&la), "Insertion Test Head Check 4");

  pos = 3; head = 5;
  la_remove(&la, pos);
  assert_int_equals(data[head], *(int *)la_head(&la), "Insertion Test Head Check 4");
}

void linked_array_tests() {
  printf("******* Linked Array Tests ********\n\n");

  basic_test();
  insertion_removal_test();

  printf("\n");
}
