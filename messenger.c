#include "queue.h"
#include "task.h"
#include "messenger.h"
#include "stdlib.h"
#include "scheduler.h"
#include <bwio.h>

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

static message message_buffer[MAX_TASKS + 1];
static messagelist receive_list[MAX_TASKS + 1];

void push_back(messagelist* ll, message* v) {
  if (ll->head == 0) {
    ll->head = v;
  } else {
    ll->tail->next = v;
  }
  ll->tail = v;
  v->next = 0;
}

message* pop_front(messagelist* ll) {
  message* ret = ll->head;
  ll->head = ll->head->next;
  ret->next = 0;
  return ret;
}

void init_messenger() {
  int i;
  for (i = 0; i < MAX_TASKS + 1; i++) {
    message_buffer[i].next = (message*) 0;
    receive_list[i].head = 0;
    receive_list[i].tail = 0;
  }
}

// Post-condition: Sender is guaranteed to either be SEND_BLOCK or REPLY_BLOCK
void messenger_send(int from, int to, char *msg, int msglen, char *reply, int replylen) {
  if (task_get_state(to) == UNUSED || task_get_state(to) == ZOMBIE) {
    task_get(from)->retval = -2;
    return;
  }

  message* letter = &(message_buffer[from]);
  letter->reply = reply;
  letter->replylen = replylen;

  message* receiver = &(message_buffer[to]);
  if (task_get_state(to) == RECV_BLCK) {
    int* tid = receiver->tid;
    *tid = from;

    int mn = min(msglen, receiver->msglen);
    memcpy(receiver->msg, msg, mn);
    Task* to_task = task_get(to);
    to_task->retval = msglen;
    tid_set_state(to, READY);
    scheduler_add_task(to_task->priority, to_task);

    tid_set_state(from, REPLY_BLCK);
    scheduler_remove_task(task_get(from)->priority);
    return;
  }

  letter->msg = msg;
  letter->msglen = msglen;
  push_back(&(receive_list[to]), letter);

  tid_set_state(from, SEND_BLCK);
  scheduler_remove_task(task_get(from)->priority);
}

int messenger_receive(int receiver, int* tid, char *msg, int msglen) {
  // check receive queue
  if (receive_list[receiver].head == 0) {
    message* inbox = &(message_buffer[receiver]);
    inbox->msg = msg;
    inbox->msglen = msglen;
    inbox->tid = tid;

    tid_set_state(receiver, RECV_BLCK);
    scheduler_remove_task(task_get(receiver)->priority);
  } else {
    message* inbox = pop_front(&(receive_list[receiver]));

    *tid = (inbox - message_buffer);
    tid_set_state(*tid, REPLY_BLCK);

    int mn = min(msglen, inbox->msglen);
    memcpy(msg, inbox->msg, mn);
    task_get(receiver)->retval = inbox->msglen;
  }

  return 0;
}

int messenger_reply(int tid, char *reply, int replylen) {
  if (task_get_state(tid) == UNUSED || task_get_state(tid) == ZOMBIE) {
    return -2;
  }
  if (task_get_state(tid) != REPLY_BLCK) {
    return -3;
  }

  message *replybox = &(message_buffer[tid]);
  int mn = min(replylen, replybox->replylen);
  memcpy(replybox->reply, reply, mn);

  Task* task = task_get(tid);
  task->retval = replylen;
  task_set_state(task, READY);
  scheduler_add_task(task->priority, task);

  return 0;
}

void messenger_incomplete(int receiver) {
  while (receive_list[receiver].head != 0) {
    message* sender_box = pop_front(&(receive_list[receiver]));
    int sender_tid = (sender_box - message_buffer);
    Task* sender_task = task_get(sender_tid);

    sender_task->retval = -3;
    task_set_state(sender_task, READY);
    scheduler_add_task(sender_task->priority, sender_task);
  }
}
