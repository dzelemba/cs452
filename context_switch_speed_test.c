#include <bwio.h>
#include <ts7200.h>

#include "all_tests.h"
#include "kernel.h"
#include "scheduler.h"
#include "syscall.h"
#include "test_helpers.h"

static void user_task_do_nothing() {
  unsigned short lasttime = 0xffff;

  int* flags = (int*)(UART1_BASE + UART_FLAG_OFFSET);
  unsigned short* timer1load = (unsigned short*)(TIMER1_BASE + LDR_OFFSET);
  while (*flags & TXFF_MASK);
  *timer1load = lasttime;

  char* timer1crtl = (char*)(TIMER1_BASE + CRTL_OFFSET);
  while ((*flags & TXFF_MASK));
  *timer1crtl = ENABLE_MASK | CLKSEL_MASK;

  volatile unsigned short* currenttimeaddr = (unsigned short*)(TIMER1_BASE + VAL_OFFSET);
  while (*currenttimeaddr == 0xffff);
  lasttime = *currenttimeaddr;

  Pass();

  unsigned short currenttime = *currenttimeaddr;

  bwprintf(COM2, "ticks: %d\n", (0xffff - currenttime));
  Exit();
}

void run_context_switch_speed_test() {
  reset_did_fail();

  Task* first_task = task_create(-1 /* Parent tid */, HI_PRI, &user_task_do_nothing);
  scheduler_add_task(HI_PRI, first_task);

  kernel_run();
  if (did_fail()) {
    bwprintf(COM2, "Context Switch Speed Test Failed!\n");
  } else {
    bwprintf(COM2, "Context Switch Speed Test Passed!\n");
  }
}
