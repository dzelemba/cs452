#include "queue.h"

int _queue_increment(int i) {
  return (i == QUEUE_BUFFER_SIZE - 1) ? 0 : i + 1;
}

int push(queue *q, int val) {
  if (is_queue_full(q)) {
    return 1;
  }
  q->buf[q->end] = val;
  q->end = _queue_increment(q->end);
  return 0;
}

int pop(queue *q) {
  int ret = q->buf[q->start];
  q->start = _queue_increment(q->start);
  return ret;
}

int is_queue_empty(queue *q) {
  return q->start == q->end;
}

int is_queue_full(queue *q) {
  return _queue_increment(q->end) == q->start;
}
