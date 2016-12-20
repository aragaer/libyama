#include <string.h>
#include <time.h>
#include "list.h"
#include "record.h"

void yama_init_record(yama_record *record, char *payload, size_t len) {
  list_init_head(&record->list);
  list_init_head(&record->log);
  record->size = len;
  int64_t stamp = time(NULL);
  record->timestamp = stamp - stamp % 60;
  memcpy(record->payload, payload, len);
}
