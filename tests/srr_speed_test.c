#include "all_tests.h"
#include "scheduler.h"
#include "kernel.h"
#include <bwio.h>
#include "syscall.h"
#include "test_helpers.h"
#include "timer.h"

/* Simple Tests */

#define ITERATIONS 1000
#define MESSAGE_LENGTH 64

static int sender_tid;
static int replyer_tid;

static void sender() {
  char buf[64];
  int i;
  int len = MESSAGE_LENGTH;
  for (i = 0; i < len - 1; i++) {
    buf[i] = i;
  }
  buf[len - 1] = 0;

  char replybuf[64];

  for (i = 0; i < ITERATIONS; i++) {
    Send(replyer_tid, buf, len, replybuf, 64);
  }

  Exit();
}

static void replyer() {
  char buf[64];
  int len = MESSAGE_LENGTH;
  int i;
  for (i = 0; i < len - 1; i++) {
    buf[i] = i;
  }
  buf[len - 1] = 0;

  char sentbuf[64];

  int tid;

  unsigned int t1 = ticks();
  for (i = 0; i < ITERATIONS; i++) {
    Receive(&tid, sentbuf, 64);
    Reply(tid, buf, len);
  }
  unsigned int t2 = ticks();
  bwprintf(COM2, "receive-block-reply ticks: %d\n", (t2 - t1));

  Exit();
}

/* Main */

static void first() {
  replyer_tid = Create(MED_PRI, &replyer);
  sender_tid = Create(LOW_PRI, &sender);
  Exit();
}

void run_srr_speed_test() {
  init_kernel();
  reset_did_fail();

  Task* first_task = task_create(-1 /* Parent tid */, HI_PRI, &first);
  scheduler_add_task(HI_PRI, first_task);

  kernel_run();

  bwprintf(COM2, "Send-Receive-Reply Performance Test Updated!\n");
}
