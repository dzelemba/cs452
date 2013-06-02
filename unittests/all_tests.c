/*
 * Main runner for tests.
 */

#include "../stdlib.h"
#include "strings_test.c"
#include "linked_array_tests.c"

int main(int argc, char** argv) {
  init_stdlib();

  strings_tests();
  linked_array_tests();

  printf("******* All Test Passed! ********\n\n");

  return 0;
}
