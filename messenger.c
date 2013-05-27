#include "queue.h"
#include "task.h"
#include "messenger.h"
#include "stdlib.h"

typedef struct _message {
  char* msg;
  int msglen;
  int* tid;
  char* reply;
  int replylen;

  struct _message* next;
} message;

typedef struct _messagelist {
  message* head;
  message* tail;
} messagelist;

static message message_buffer[MAX_TASKS];
static messagelist receive_list[MAX_TASKS];

void push_back(messagelist* ll, message* v) {
  if (ll->head == 0) {
    ll->head = v;
    ll->tail->next = v;
  }
  ll->tail = v;
}

message* pop_front(messagelist* ll) {
  message* ret = ll->head;
  ll->head = ll->head->next;
  return ret;
}

void init_messenger() {
  int i;
  for (i = 0; i < MAX_TASKS; i++) {
    message_buffer[i].next = (message*) 0;
    receive_list[i].head = 0;
    receive_list[i].tail = 0;
  }
}

// Post-condition: Sender is guaranteed to either be SEND_BLOCK or REPLY_BLOCK
int messenger_send(int from, int to, char *msg, int msglen, char *reply, int replylen) {
  if (task_get_state(to) == UNUSED) {
    return -2;
  }

  message* receiver = &(message_buffer[to]);
  if (task_get_state(to) == RECV_BLCK) {
    int* tid = receiver->tid;
    *tid = from;

    int mn = min(msglen, receiver->msglen);
    memcpy(receiver->msg, msg, mn);
    task_get(to)->retval = msglen;

    tid_set_state(from, REPLY_BLCK);
    return 0;
  }

  message* letter = &(message_buffer[from]);
  letter->msg = msg;
  letter->msglen = msglen;
  push_back(&(receive_list[to]), letter);

  tid_set_state(from, SEND_BLCK);

  return 0;
}

int messenger_receive(int to, int* tid, char *msg, int msglen) {
  // check receive queue
  if (receive_list[to].head == 0) {
    message* inbox = &(message_buffer[to]);
    inbox->msg = msg;
    inbox->msglen = msglen;
    inbox->tid = tid;

    tid_set_state(to, RECV_BLCK);
  } else {
    message* inbox = pop_front(&(receive_list[to]));

    int mn = min(msglen, inbox->msglen);
    memcpy(msg, inbox->msg, mn);
    task_get(to)->retval = inbox->msglen;
  }

  return 0;
}

int messenger_reply(int tid, char *reply, int replylen) {
  if (task_get_state(tid) != REPLY_BLCK) {
    return -3;
  }

  message *replybox = &(message_buffer[tid]);
  int mn = min(replylen, replybox->replylen);
  memcpy(replybox->reply, reply, mn);
  task_get(tid)->retval = replylen;

  return 0;
}
