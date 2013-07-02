#include "calibration.h"
#include "kernel.h"
#include "priorities.h"
#include "run_tests.h"

int main(int argc, char** argv) {

#ifdef TEST
  run_tests();
#else
#ifdef CALIB
  init_kernel();
  kernel_add_task(MED_PRI, &calibration_task);
  kernel_run();
#else
  init_kernel();
  kernel_run();
#endif
#endif

  return 0;
}
