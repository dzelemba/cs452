#include "all_tests.h"
#include "scheduler.h"
#include "kernel.h"
#include <bwio.h>
#include "syscall.h"
#include "test_helpers.h"

#define NUM_CLIENTS 8
#define MESSAGES_PER_CLIENT 2

/* Simple Tests */

static int simple_client_tid;
static int simple_server_tid;
static char* simple_message;
static char* simple_reply;

static void simple_client() {
  char reply[8];

  Send(simple_server_tid, simple_message, 2, reply, 8);
  assert_char_equals(simple_reply[0], reply[0], "Message Passing Test: Simple Client");

  Exit();
}

static void simple_server() {
  int tid;
  char msg[8];

  Receive(&tid, msg, 8);
  assert_int_equals(simple_client_tid, tid, "Message Passing Test: Simple Server Tid");
  assert_char_equals(simple_message[0], msg[0], "Message Passing Test: Simple Server Msg");

  Reply(tid, simple_reply, 2);

  Exit();
}

static void simple_test() {
  simple_client_tid = -1;
  simple_server_tid = -2;
  simple_message = "a";
  simple_reply = "b";

  simple_server_tid = Create(MED_PRI, &simple_server);
  simple_client_tid = Create(MED_PRI, &simple_client);

  Exit();
}

/* Multiple Clients Test */

static int server_tid;

static void client() {
  char message[2];
  char reply[16];

  int tid = MyTid();
  message[0] = tid;
  message[1] = '\0';

  int i;
  for (i = 0; i < MESSAGES_PER_CLIENT; i++) {
    Send(server_tid, message, 2, reply, 16);
    assert_char_equals((char)tid, reply[0], "Message Passing Test: Multiplie Clients Test: Client");
  }

  Exit();
}

static void server() {
  char buf[16];
  int tid;

  int i;
  for (i = 0; i < NUM_CLIENTS * MESSAGES_PER_CLIENT; i++) {
    Receive(&tid, buf, 16);
    assert_char_equals((char)tid, buf[0], "Message Passing Test: Multiple Clients Test: Server");
    Reply(tid, buf, 16);
  }

  Exit();
}

static void multiple_clients_test() {
  server_tid = Create(MED_PRI, &server);

  int i;
  for (i = 0; i < NUM_CLIENTS; i++) {
    Create(MED_PRI, &client);
  }

  Exit();
}


/* Main */

static void first() {
  Create(HI_PRI, &simple_test);
  Create(HI_PRI, &multiple_clients_test);

  Exit();
}

void run_message_passing_test() {
  init_kernel();
  reset_did_fail();

  Task* first_task = task_create(-1 /* Parent tid */, LOW_PRI, &first);
  scheduler_add_task(LOW_PRI, first_task);

  kernel_run();

  if (did_fail()) {
    bwprintf(COM2, "Message Passing Test Failed!\n");
  } else {
    bwprintf(COM2, "Message Passing Test Passed!\n");
  }
}
