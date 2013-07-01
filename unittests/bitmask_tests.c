#include "bitmask.h"
#include "test_helpers.h"
#include <stdio.h>

#define SIZE 10

static void set_unset_test() {
  printf("Set/Unset Tests\n");

  bitmask bm;
  bm_create(&bm);

  int bit, hi, low;

  hi = 0; low = 0;
  assert_int_equals(hi, bm_gethibit(&bm), "Basic Test Hi 1");
  assert_int_equals(low, bm_getlowbit(&bm), "Basic Test Low 1");

  bit = 5; hi = 5; low = 5;
  bm_set(&bm, bit);
  assert_int_equals(hi, bm_gethibit(&bm), "Basic Test Hi 1");
  assert_int_equals(low, bm_getlowbit(&bm), "Basic Test Low 1");
  assert_true(bm_isset(&bm, bit), "Basic Test isset 1");

  bit = 10; hi = 10; low = 5;
  bm_set(&bm, bit);
  assert_int_equals(hi, bm_gethibit(&bm), "Basic Test Hi 2");
  assert_int_equals(low, bm_getlowbit(&bm), "Basic Test Low 2");
  assert_true(bm_isset(&bm, bit), "Basic Test isset 2");

  bit = 7; hi = 10; low = 5;
  bm_set(&bm, bit);
  assert_int_equals(hi, bm_gethibit(&bm), "Basic Test Hi 3");
  assert_int_equals(low, bm_getlowbit(&bm), "Basic Test Low 2");
  assert_true(bm_isset(&bm, bit), "Basic Test isset 3");

  bit = 5; hi = 10; low = 7;
  bm_unset(&bm, bit);
  assert_int_equals(hi, bm_gethibit(&bm), "Basic Test Hi 3");
  assert_int_equals(low, bm_getlowbit(&bm), "Basic Test Low 2");
  assert_false(bm_isset(&bm, bit), "Basic Test isset 4");
}

void bitmask_tests() {
  printf("******* Bitmask Tests ********\n\n");

  set_unset_test();

  printf("\n");
}
