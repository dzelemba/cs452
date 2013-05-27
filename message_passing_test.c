#include "all_tests.h"
#include "scheduler.h"
#include "kernel.h"
#include <bwio.h>
#include "syscall.h"
#include "test_helpers.h"

#define NUM_CLIENTS 8
#define MESSAGES_PER_CLIENT 2

int server_tid;

static void client() {
  char* message = "a";
  char reply[16];

  int tid = MyTid();
  message[0] = tid;

  int i;
  for (i = 0; i < MESSAGES_PER_CLIENT; i++) {
    Send(server_tid, message, 2, reply, 16);
    assert_char_equals((char)tid, reply[0], "Message Passing Test: Client");
  }

  Exit();
}

static void server() {
  char buf[16];
  int* tid;

  int i;
  for (i = 0; i < NUM_CLIENTS * MESSAGES_PER_CLIENT; i++) {
    Receive(tid, buf, 16);
    assert_char_equals((char)*tid, buf[0], "Message Passing Test: Server");
    Reply(*tid, buf, 16);
  }

  Exit();
}

static void first() {
  server_tid = Create(HI_PRI, &server);

  int i;
  for (i = 0; i < NUM_CLIENTS: i++) {
    Create(HI_PRI, &client);
  }
}

void run_basic_test() {
  init_kernel();
  reset_did_fail();

  Task* first_task = task_create(-1 /* Parent tid */, MED_PRI, &first);
  scheduler_add_task(MED_PRI, first_task);

  kernel_run();

  if (did_fail()) {
    bwprintf(COM2, "Message Passing Test Failed!\n");
  } else {
    bwprintf(COM2, "Message Passing Test Passed!\n");
  }
}
