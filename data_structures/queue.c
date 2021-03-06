#include "queue.h"
#include "debug.h"

void init_queue(queue *q, int* buf, int size) {
  q->start = 0;
  q->end = 0;
  q->buf = buf;
  q->size = size;
}

void q_set_name(queue *q, char * name) {
  q->name = name;
}

static inline int _queue_increment(queue* q, int i) {
  return (i == q->size - 1) ? 0 : i + 1;
}

void push(queue *q, int val) {
  ASSERT(!is_queue_full(q), "queue.c: queue %s push: queue full", q->name);

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

void queue_get_mem(queue* q, int** mem, int* size) {
  *mem = (q->buf + q->start);
  *size = queue_size(q);
}

void queue_clear(queue* q) {
  q->start = 0;
  q->end = 0;
}

// For iterating over the queue.

void init_queue_iterator(queue* q, queue_iterator* q_it) {
  q_it->pos = q->start;
}

int qit_has_next(queue* q, queue_iterator* q_it) {
  return q->end != q_it->pos;
}

int qit_get_next(queue* q, queue_iterator* q_it) {
  ASSERT(qit_has_next(q, q_it), "queue.c: queue: %s qit_has_next", q->name);
  int ret_val = q->buf[q_it->pos];
  q_it->pos = _queue_increment(q, q_it->pos);
  return ret_val;
}
