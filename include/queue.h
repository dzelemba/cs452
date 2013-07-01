#ifndef __QUEUE_H__
#define __QUEUE_H__

typedef struct _queue {
  int start;
  int end;
  int* buf;
  int size;
} queue;

void init_queue(queue *q, int* buf, int size);

void push(queue *q, int val);

int pop(queue *q);

int head(queue *q);

int is_queue_empty(queue *q);

int is_queue_full(queue *q);

int queue_size(queue *q);

// For iterating over the queue.

typedef struct queue_iterator {
  int pos;
} queue_iterator;

void init_queue_iterator(queue* q, queue_iterator* q_it);

int qit_has_next(queue* q, queue_iterator* q_it);

int qit_get_next(queue* q, queue_iterator* q_it);

#endif
