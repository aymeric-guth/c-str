#include <stdio.h>

#include "logging.h"
#include "str.h"

extern int running;

void *kb_input(void *arg) {
  Q *q = (Q *)arg;

  while (running) {
    str val = Q_get(q);

    if (val != NULL)
      printf("log\n");
  }

  return NULL;
}

str Q_get(Q *q) {
  if (q->tail == q->head)
    return NULL;

  *retval = q->data[q->tail];
  q->data[q->tail] = 0;
  q->tail = (q->tail + 1) % q->size;
  return 1;
}

int Q_put(Q *q, str *val) {
  if (((q->head + 1) % q->size) == q->tail)
    return 0;

  q->data[q->head] = val;
  q->head = (q->head + 1) % q->size;
  return 1;
}
