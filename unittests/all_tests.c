/*
 * Main runner for tests.
 */

#include "../stdlib.h"
#include "strings_tests.c"
#include "linked_array_tests.c"
#include "bitmask_tests.c"
#include "heap_tests.c"

int main(int argc, char** argv) {
  init_stdlib();

  strings_tests();
  linked_array_tests();
  bitmask_tests();
  heap_tests();

  printf("******* All Test Passed! ********\n\n");

  return 0;
}
