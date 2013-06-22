#include "queue.h"
#include "debug.h"

void init_queue(queue *q, int* buf, int size) {
  q->start = 0;
  q->end = 0;
  q->buf = buf;
  q->size = size;
}

static inline int _queue_increment(queue* q, int i) {
  return (i == q->size - 1) ? 0 : i + 1;
}

void push(queue *q, int val) {
  ASSERT(!is_queue_full(q), "queue.c: push: queue full");

  q->buf[q->end] = val;
  q->end = _queue_increment(q, q->end);
}

int pop(queue *q) {
  int ret = q->buf[q->start];
  q->start = _queue_increment(q, q->start);
  return ret;
}

int head(queue *q) {
  return q->buf[q->start];
}

int is_queue_empty(queue *q) {
  return q->start == q->end;
}

int is_queue_full(queue *q) {
  return _queue_increment(q, q->end) == q->start;
}

int queue_size(queue *q) {
  return q->end - q->start;
}
