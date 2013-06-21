#include "events.h"
#include "icu.h"
#include "ioserver.h"
#include "priorities.h"
#include "queue.h"
#include "syscall.h"
#include "task.h"
#include "uart.h"

#define REQUEST_TRAIN_GETC  0
#define REQUEST_TRAIN_PUTC  1
#define NOTIF_TRAIN_HASDATA 2
#define NOTIF_TRAIN_CANPUT  3
#define REQUEST_TERM_GETC   4
#define REQUEST_TERM_PUTC   5
#define NOTIF_TERM_HASDATA  6
#define NOTIF_TERM_CANPUT   7

typedef struct ioserver_request {
  char type;
  char msg;
} ioserver_request;

static int ioserver_tid;
static queue train_data_queue, train_getc_queue, train_putc_queue, term_data_queue, term_getc_queue, term_putc_queue;

void uart1_write_notifier_run() {
  ioserver_request req = { NOTIF_TRAIN_CANPUT, 0 };

  while (1) {
    AwaitEvent(EVENT_UART1_TX_READY);
    Send(ioserver_tid, (char *)&req, sizeof(ioserver_request), (void *)0, 0);
  }

  Exit();
}

void uart1_read_notifier_run() {
  ioserver_request req = { NOTIF_TRAIN_HASDATA, 0 };

  while (1) {
    req.msg = AwaitEvent(EVENT_UART1_RCV_READY);
    Send(ioserver_tid, (char *)&req, sizeof(ioserver_request), (void *)0, 0);
  }

  Exit();
}

void uart2_write_notifier_run() {
  ioserver_request req = { NOTIF_TERM_CANPUT, 0 };

  while (1) {
    AwaitEvent(EVENT_UART2_TX_READY);
    Send(ioserver_tid, (char *)&req, sizeof(ioserver_request), (void *)0, 0);
  }

  Exit();
}

void uart2_read_notifier_run() {
  ioserver_request req = { NOTIF_TERM_HASDATA, 0 };

  while (1) {
    req.msg = AwaitEvent(EVENT_UART2_RCV_READY);
    Send(ioserver_tid, (char *)&req, sizeof(ioserver_request), (void *)0, 0);
  }

  Exit();
}

void ioserver_run() {
  // TODO: The data queues don't make sense to be bounded by MAX_TASKS

  init_queue(&train_getc_queue); // tid
  init_queue(&train_data_queue); // char
  init_queue(&train_putc_queue); // char
  init_queue(&term_getc_queue); // tid
  init_queue(&term_data_queue); // char
  init_queue(&term_putc_queue); // char

  Create(MAX_PRI, &uart1_write_notifier_run);
  Create(MAX_PRI, &uart1_read_notifier_run);
  Create(MAX_PRI, &uart2_write_notifier_run);
  Create(MAX_PRI, &uart2_read_notifier_run);

  int tid;
  ioserver_request req;
  char train_can_put = 0, term_can_put = 0;

  // Gross. A bunch of local variables used across our switch statement
  int qgetc_tid;
  char ch;

  while (1) {
    Receive(&tid, (char *)&req, sizeof(ioserver_request));
    switch (req.type) {
      case NOTIF_TRAIN_CANPUT:
        if (!is_queue_empty(&train_putc_queue)) {
          ch = pop(&train_putc_queue);
          ua_putc(COM1, ch);
        } else {
          train_can_put = 1;
        }

        Reply(tid, (void *)0, 0);
        break;

      case NOTIF_TRAIN_HASDATA:
        if (!is_queue_empty(&train_getc_queue)) {
          qgetc_tid = pop(&train_getc_queue);
          Reply(qgetc_tid, &req.msg, sizeof(char));
        } else {
          push(&train_data_queue, req.msg);
        }

        Reply(tid, (void *)0, 0);
        break;

      case REQUEST_TRAIN_GETC:
        if (!is_queue_empty(&train_data_queue)) {
          ch = pop(&train_data_queue);
          Reply(tid, &ch, sizeof(char));
        } else {
          push(&train_getc_queue, tid);
        }
        break;

      case REQUEST_TRAIN_PUTC:
        if (train_can_put) {
          ua_putc(COM1, req.msg);
          train_can_put = 0;
        } else {
          push(&train_putc_queue, req.msg);
        }

        Reply(tid, (void *)0, 0);
        break;

      case NOTIF_TERM_CANPUT:
        if (!is_queue_empty(&term_putc_queue)) {
          ch = pop(&term_putc_queue);
          ua_putc(COM2, ch);
        } else {
          term_can_put = 1;
        }

        Reply(tid, (void *)0, 0);
        break;

      case NOTIF_TERM_HASDATA:
        if (!is_queue_empty(&term_getc_queue)) {
          qgetc_tid = pop(&term_getc_queue);
          Reply(qgetc_tid, &req.msg, sizeof(char));
        } else {
          push(&term_data_queue, req.msg);
        }

        Reply(tid, (void *)0, 0);
        break;

      case REQUEST_TERM_GETC:
        if (!is_queue_empty(&term_data_queue)) {
          ch = pop(&term_data_queue);
          Reply(tid, &ch, sizeof(char));
        } else {
          push(&term_getc_queue, tid);
        }
        break;

      case REQUEST_TERM_PUTC:
        if (term_can_put) {
          ua_putc(COM2, req.msg);
          term_can_put = 0;
        } else {
          push(&term_putc_queue, req.msg);
        }

        Reply(tid, (void *)0, 0);
        break;
    }
  }

  Exit();
}

/* Public Methods */

void start_ioserver() {
  ioserver_tid = Create(MAX_PRI, &ioserver_run);
}

// TODO: New constants instead of this COM1, COM2 stuff

int Getc(int channel) {
  char ch;
  ioserver_request req;
  req.type = (channel == COM1) ? REQUEST_TRAIN_GETC : REQUEST_TERM_GETC;
  Send(ioserver_tid, (char *)&req, sizeof(ioserver_request), &ch, sizeof(char));
  return ch;
}

int Putc(int channel, char ch) {
  ioserver_request req;
  req.type = (channel == COM1) ? REQUEST_TRAIN_PUTC : REQUEST_TERM_PUTC;
  req.msg = ch;
  Send(ioserver_tid, (char *)&req, sizeof(ioserver_request), (void *)0, 0);
  return 0;
}
