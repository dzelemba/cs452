#include "all_tests.h"
#include "bwio.h"
#include "kernel.h"
#include "priorities.h"
#include "syscall.h"
#include "task.h"
#include "test_helpers.h"
#include "ts7200.h"

static void first() {
  /*Putc(COM1, 133); // reads sensor data*/
  int i;
  /*for (i = 0; i < 10; i++) {*/
    /*char ch = Getc(COM1);*/
  /*}*/

  /*Putc(COM1, 10);*/
  /*Putc(COM1, 49);*/

  /*char ch = Getc(COM2);*/
  /*Putc(COM2, ch + 1);*/
  char kk = Getc(COM2);
  Putc(COM2, kk);

  Putc(COM2, 'f');
  Putc(COM2, 'e');
  Putc(COM2, 'l');
  Putc(COM2, 'i');
  Putc(COM2, 'x');
  Putc(COM2, 'g');
  Putc(COM2, '\n');

  char ch = Getc(COM2);

  Exit();
}

void run_read_sensors_test() {
  init_kernel();
  reset_did_fail();

  kernel_add_task(VLOW_PRI, &first);

  kernel_run();

  if (did_fail()) {
    /*bwprintf(COM2, "Read Sensors Test Failed!\n");*/
  } else {
    /*bwprintf(COM2, "Read Sensors Test Passed!\n");*/
  }
}
