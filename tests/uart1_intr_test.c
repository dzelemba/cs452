#include "all_tests.h"
#include "bwio.h"
#include "kernel.h"
#include "priorities.h"
#include "syscall.h"
#include "task.h"
#include "test_helpers.h"
#include "events.h"
#include "uart.h"
#include "debug.h"

/*
 * This will probably result in some missed events, thats ok.
 */

static void writerHelper() {
  ua_putc(COM1, 128 + 5);
  USER_INFO("Writer Send Byte\n");

  Exit();
}

static void writer() {
  AwaitEvent(EVENT_UART1_TX_READY);
  Create(LOW_PRI, &writerHelper);

  USER_INFO("Writer Done\b");
  Exit();
}

static void readerHelper() {
  USER_INFO("Char read\n");

  Exit();
}

static void reader() {
  int i;
  for (i = 0; i < 10; i++) {
    AwaitEvent(EVENT_UART1_RCV_READY);
    Create(LOW_PRI, &readerHelper);
  }

  USER_INFO("Reader Done\b");
  Exit();
}

static void first() {
  Create(MED_PRI, &writer);
  Create(MED_PRI, &reader);

  Exit();
}

void run_uart1_intr_test() {
  init_kernel();
  reset_did_fail();

  kernel_add_task(VLOW_PRI, &first);

  kernel_run();

  if (did_fail()) {
    bwprintf(COM2, "Serial IO Test Failed!\n");
  } else {
    bwprintf(COM2, "Serial IO Test Passed!\n");
  }
}
