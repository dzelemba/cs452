#include "all_tests.h"
#include "kernel.h"
#include <bwio.h>
#include "syscall.h"
#include "test_helpers.h"
#include "priorities.h"

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

/* Send & Receive Tests */

// Testing both sending & receiving in each user task.

#define MESSAGE_SIZE 12
#define ITERATIONS 10

static int task_1_tid;
static int task_2_tid;
static char* message_1;
static char* message_2;

static void task_1() {
  char reply[MESSAGE_SIZE];
  int tid;

  int i;
  for (i = 0; i < ITERATIONS; i++) {
    Send(task_2_tid, message_1, MESSAGE_SIZE, reply, MESSAGE_SIZE);
    assert_string_equals(message_2, reply, "Message Passing Test: Send Receive");

    Receive(&tid, reply, MESSAGE_SIZE);
    assert_int_equals(task_2_tid, tid, "Message Passing Test: Send Receive Task 1 Check Tid");
    assert_string_equals(message_2, reply, "Message Passing Test: Send Receive Task 1 Check Reply");
    Reply(tid, message_1, MESSAGE_SIZE);
  }

  Exit();
}

static void task_2() {
  char reply[MESSAGE_SIZE];
  int tid;

  int i;
  for (i = 0; i < ITERATIONS; i++) {
    Receive(&tid, reply, MESSAGE_SIZE);
    assert_int_equals(task_1_tid, tid, "Message Passing Test: Send Receive");
    assert_string_equals(message_1, reply, "Message Passing Test: Send Receive");
    Reply(tid, message_2, MESSAGE_SIZE);

    Send(task_1_tid, message_2, MESSAGE_SIZE, reply, MESSAGE_SIZE);
    assert_string_equals(message_1, reply, "Message Passing Test: Send Receive Check Send Reply");
  }

  Exit();
}

static void send_receive_tests() {
  message_1 = "message 1";
  message_2 = "message 2";

  task_1_tid = Create(MED_PRI, &task_1);
  task_2_tid = Create(MED_PRI, &task_2);

  Exit();
}

/* Main */

static void first() {
  Create(HI_PRI, &simple_test);
  Create(HI_PRI, &multiple_clients_test);
  Create(HI_PRI, &send_receive_tests);

  Exit();
}

void run_message_passing_test() {
  init_kernel();
  reset_did_fail();

  kernel_add_task(LOW_PRI, &first);

  kernel_run();

  if (did_fail()) {
    bwprintf(COM2, "Message Passing Test Failed!\n");
  } else {
    bwprintf(COM2, "Message Passing Test Passed!\n");
  }
}
