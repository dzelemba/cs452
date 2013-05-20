#include "queue.h"

int _queue_increment(int i) {
  return (i == QUEUE_BUFFER_SIZE - 1) ? 0 : i + 1;
}

void push(queue *q, int val) {
  q->buf[q->end] = val;
  q->end = _queue_increment(q->end);
}

int pop(queue *q) {
  int ret = q->buf[q->start];
  q->start = _queue_increment(q->start);
  return ret;
}

int head(queue *q) {
  return q->buf[q->start];
}

int is_queue_empty(queue *q) {
  return q->start == q->end;
}

int is_queue_full(queue *q) {
  return _queue_increment(q->end) == q->start;
}
