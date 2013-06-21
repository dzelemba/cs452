#include "kernel.h"
#include "run_tests.h"
#include "stdio.h"

int main(int argc, char** argv) {
  run_tests();

  printf(COM2, "Main Exiting... \n");

  return 0;
}
