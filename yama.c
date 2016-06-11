#include <stdlib.h>
#include <string.h>
#include "yama.h"

static char _magic[4] = {'Y', 'A', 'M', 'A'};

YAMA *yama_new() {
  YAMA *result = malloc(sizeof(YAMA));
  memset(result, 0, sizeof(*result));
  return memcpy(result, _magic, sizeof(_magic));
}

void yama_release(YAMA *header) {
  yama_record *item, *next;
  item = header->first;
  while (item != NULL) {
    next = item->next;
    free(item);
    item = next;
  }
  free(header);
}

yama_record *yama_first(const YAMA *header) {
  return header->first;
}

yama_record *yama_next(const YAMA *header,
		       yama_record *item) {
  return item->next;
}

yama_record *yama_add(YAMA * const header,
		      const char *payload) {
  yama_record *result;
  int datalen = strlen(payload);
  result = malloc(sizeof(yama_record) + datalen);
  result->size = datalen;
  memcpy(result->payload, payload, datalen);
  result->next = header->first;
  header->first = result;
  return result;
}
