#ifndef __QUEUE_H__
#define __QUEUE_H__

#define QUEUE_BUFFER_SIZE (1024)

typedef struct _queue {
  int start;
  int end;
  int buf[1024];
} queue;

void push(queue *q, int val);

int pop(queue *q);

int is_queue_empty(queue *q);

#endif
