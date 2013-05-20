#include <bwio.h>
#include "kernel.h"
#include "run_tests.h"

int main(int argc, char** argv) {
  init_kernel();

  run_tests();

  bwprintf(COM2, "Main Exiting... \n");

  return 0;
}
