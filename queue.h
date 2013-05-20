#ifndef __QUEUE_H__
#define __QUEUE_H__

#define QUEUE_BUFFER_SIZE (1024)

typedef struct _queue {
  int start;
  int end;
  int buf[QUEUE_BUFFER_SIZE];
} queue;

void push(queue *q, int val);

int pop(queue *q);

int head(queue *q);

int is_queue_empty(queue *q);

int is_queue_full(queue *q);

#endif
