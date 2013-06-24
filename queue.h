#ifndef __QUEUE_H__
#define __QUEUE_H__

#define QUEUE_MAX_ITEMS (127)

typedef struct _queue {
  int start;
  int end;
  int buf[QUEUE_MAX_ITEMS + 1];
} queue;

void init_queue(queue *q);

void push(queue *q, int val);

int pop(queue *q);

int head(queue *q);

int is_queue_empty(queue *q);

int is_queue_full(queue *q);

int queue_size(queue *q);

#endif
