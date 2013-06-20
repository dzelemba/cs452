#include "events.h"
#include "icu.h"
#include "ioserver.h"
#include "priorities.h"
#include "queue.h"
#include "syscall.h"
#include "task.h"
#include "uart.h"

#include "bwio.h"

#define REQUEST_GETC 0
#define REQUEST_PUTC 1
#define REQUEST_NOTIF_WRITE 2
#define REQUEST_NOTIF_READ 3

typedef struct ioserver_request {
  char type;
  char msg;
} ioserver_request;

static int train_ioserver_tid, term_ioserver_tid;
static queue data_queue, getc_queue, putc_queue;

void uart1_write_notifier_run() {
  ioserver_request req = { REQUEST_NOTIF_WRITE, 0 };

  while (1) {
    AwaitEvent(EVENT_UART1_TX_READY);
    Send(train_ioserver_tid, (char *)&req, sizeof(ioserver_request), (void *)0, 0);
  }

  Exit();
}

void uart1_read_notifier_run() {
  ioserver_request req = { REQUEST_NOTIF_READ, 0 };

  while (1) {
    req.msg = AwaitEvent(EVENT_UART1_RCV_READY);
    Send(train_ioserver_tid, (char *)&req, sizeof(ioserver_request), (void *)0, 0);
  }

  Exit();
}

void ioserver_run() {
  // Because what is even typing?
  init_queue(&getc_queue); // tid
  init_queue(&putc_queue); // char
  init_queue(&data_queue); // char

  Create(MAX_PRI, &uart1_write_notifier_run);
  Create(MAX_PRI, &uart1_read_notifier_run);

  int tid;
  ioserver_request req;
  char can_write = 0;

  // Gross. A bunch of local variables used across our switch statement
  int qgetc_tid;
  char ch;

  while (1) {
    Receive(&tid, (char *)&req, sizeof(ioserver_request));
    switch (req.type) {
      case REQUEST_NOTIF_WRITE:
        if (!is_queue_empty(&putc_queue)) {
          ch = pop(&putc_queue);
          ua_putc(COM1, ch);
        } else {
          can_write = 1;
        }

        Reply(tid, (void *)0, 0);
        break;

      case REQUEST_NOTIF_READ:
        ch = ua_getc(COM1);

        if (!is_queue_empty(&getc_queue)) {
          qgetc_tid = pop(&getc_queue);
          Reply(qgetc_tid, &ch, sizeof(char));
        } else {
          push(&data_queue, ch);
        }

        Reply(tid, (void *)0, 0);
        break;

      case REQUEST_GETC:
        if (!is_queue_empty(&data_queue)) {
          ch = pop(&data_queue);
          Reply(tid, &ch, sizeof(char));
        } else {
          push(&getc_queue, tid);
        }
        break;

      case REQUEST_PUTC:
        if (can_write) {
          ua_putc(COM1, req.msg);
          can_write = 0;
        } else {
          push(&putc_queue, req.msg);
        }

        Reply(tid, (void *)0, 0);
        break;
    }
  }

  Exit();
}

/* Public Methods */

void start_ioserver() {
  train_ioserver_tid = Create(MAX_PRI, &ioserver_run);
}

int Getc(int channel) {
  char ch;
  ioserver_request req;
  req.type = REQUEST_GETC;

  if (channel == COM1) {
    Send(train_ioserver_tid, (char *)&req, sizeof(ioserver_request), &ch, sizeof(char));
    return ch;
  } else {
    return -1; // Not implemented
  }
}

int Putc(int channel, char ch) {
  ioserver_request req;
  req.type = REQUEST_PUTC;
  req.msg = ch;

  if (channel == COM1) {
    Send(train_ioserver_tid, (char *)&req, sizeof(ioserver_request), (void *)0, 0);
    return 0;
  } else {
    return -1; // Not implemented
  }
}
