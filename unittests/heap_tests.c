#include "../heap.h"
#include "test_helpers.h"
#include <stdio.h>

static void simple_heap_test() {
  printf("Simple Heap\n");

  heap hp;
  heap_node mem[64];
  init_heap(&hp, mem, 64);

  char a = 'a', b = 'b', c = 'c', d = 'd';
  heap_insert(&hp, 1, &b);
  heap_insert(&hp, 100, &d);
  heap_insert(&hp, 0, &a);
  heap_insert(&hp, 5, &c);

  char ret = *(char*) heap_min_value(&hp);
  assert_equals(ret, 'a', "Simple Heap Min 1");

  ret = *(char*) heap_delete_min(&hp);
  assert_equals(ret, 'a', "Simple Heap Delete 1");

  ret = *(char*) heap_min_value(&hp);
  assert_equals(ret, 'b', "Simple Heap Min 2");

  ret = *(char*) heap_delete_min(&hp);
  assert_equals(ret, 'b', "Simple Heap Delete 2");

  ret = *(char*) heap_delete_min(&hp);
  assert_equals(ret, 'c', "Simple Heap Delete 3");

  ret = *(char*) heap_delete_min(&hp);
  assert_equals(ret, 'd', "Simple Heap Delete 4");
}

static void easy_heap_test() {
  printf("Easy Heap\n");

  heap hp;
  heap_node mem[64];
  init_heap(&hp, mem, 64);

  char f = 'f', g = 'g', h = 'h', i = 'i';
  heap_insert(&hp, 12, &i);
  heap_insert(&hp, 7, &h);

  char ret = *(char*) heap_delete_min(&hp);
  assert_equals(ret, 'h', "Easy Heap Delete 1");

  heap_insert(&hp, 1, &f);
  heap_insert(&hp, 2, &g);

  ret = *(char*) heap_delete_min(&hp);
  assert_equals(ret, 'f', "Easy Heap Delete 2");
  ret = *(char*) heap_delete_min(&hp);
  assert_equals(ret, 'g', "Easy Heap Delete 3");
  ret = *(char*) heap_delete_min(&hp);
  assert_equals(ret, 'i', "Easy Heap Delete 4");
}

static void emptying_heap_test() {
  printf("Emptying Heap\n");

  heap hp;
  heap_node mem[64];
  init_heap(&hp, mem, 64);

  char a = 'a', b = 'b';
  heap_insert(&hp, 1, &a);

  char ret = *(char*) heap_delete_min(&hp);
  assert_equals(ret, 'a', "Emptying Heap Delete 1");

  heap_insert(&hp, 1, &b);
  ret = *(char*) heap_delete_min(&hp);
  assert_equals(ret, 'b', "Emptying Heap Delete 2");
}

static void same_priority_test() {
  printf("Same Priority\n");

  char a = 'a', b = 'b', c = 'c';

  heap hp;
  heap_node mem[64];
  init_heap(&hp, mem, 64);

  heap_insert(&hp, 1, &a);
  heap_insert(&hp, 1, &b);
  heap_insert(&hp, 3, &c);

  char ret1 = *(char*) heap_delete_min(&hp);
  char ret2 = *(char*) heap_delete_min(&hp);

  assert_true((ret1 == 'a' && ret2 == 'b') || (ret1 == 'b' && ret2 == 'a'), "Same Priority Delete 1");

  char ret = *(char*) heap_delete_min(&hp);
  assert_equals(ret, 'c', "Same Priority Delete 2");
}

void heap_tests() {
  printf("******* Heap Tests ********\n\n");

  simple_heap_test();
  easy_heap_test();
  emptying_heap_test();
  same_priority_test();

  printf("\n");
}
