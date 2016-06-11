#include <stdlib.h>
#include <string.h>
#include "yama.h"

static char _magic[4] = {'Y', 'A', 'M', 'A'};

yama_header *yama_new() {
  yama_header *result = malloc(sizeof(yama_header));
  return memcpy(result, _magic, sizeof(_magic));
}

void *yama_first(const yama_header *header) {
  return header->first;
}

yama_record *yama_add(yama_header * const header,
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
