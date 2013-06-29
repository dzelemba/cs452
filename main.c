#include "kernel.h"
#include "run_tests.h"

int main(int argc, char** argv) {

#ifndef TEST
  init_kernel();
  kernel_run();
#else
  run_tests();
#endif

  return 0;
}
