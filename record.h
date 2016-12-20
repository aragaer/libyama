#ifndef _RECORD_H_
#define _RECORD_H_

#include <stddef.h>
#include "list.h"

#pragma GCC visibility push(internal)

typedef struct __attribute__((packed, aligned(4))) {
  int32_t size;
  list_head list;
  list_head log;
  int done: 1;
  int64_t timestamp;
  char payload[0];
} yama_record;

static inline yama_record *list_item_to_record(list_head const * const item) {
  return item == NULL
    ? NULL
    : container_of(item, yama_record, list);
}

void yama_init_record(yama_record *record, char *payload, size_t len);

#pragma GCC visibility pop

#endif  // _RECORD_H_
