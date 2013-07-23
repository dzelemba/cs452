/*
 * Main runner for tests.
 */

#include "ourlib.h"
#include "strings_tests.c"
#include "linked_array_tests.c"
#include "bitmask_tests.c"
#include "heap_tests.c"
#include "dijkstra_tests.c"
#include "free_tests.c"

int main(int argc, char** argv) {
  init_stdlib();

  strings_tests();
  linked_array_tests();
  bitmask_tests();
  heap_tests();
  dijkstra_tests();
  free_tests();

  printf("******* All Test Passed! ********\n\n");

  return 0;
}
