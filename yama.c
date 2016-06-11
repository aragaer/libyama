#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "yama.h"

static char _magic[4] = {'Y', 'A', 'M', 'A'};

YAMA *yama_new() {
  YAMA *result = malloc(sizeof(YAMA));
  memset(result, 0, sizeof(*result));
  memcpy(result, _magic, sizeof(_magic));
  result->payload = malloc(sizeof(yama_payload));
  memset(result->payload, 0, sizeof(yama_payload));
  result->first = -1;
  return result;
}

YAMA *yama_read(int fd) {
  YAMA *result = yama_new();
  write(fd, result, sizeof(*result));
  return result;
}

void yama_release(YAMA *header) {
  free(header->payload);
  free(header);
}

static inline yama_record *offt_to_record(const YAMA *yama,
					  uint32_t offt) {
  if (offt == -1)
    return NULL;
  char *ptr = yama->payload->payload + offt;
  return (yama_record *) ptr;
}

yama_record *yama_first(const YAMA *yama) {
  return offt_to_record(yama, yama->first);
}

yama_record *yama_next(const YAMA *yama,
		       yama_record *item) {
  return offt_to_record(yama, item->next);
}

yama_record *yama_add(YAMA * const yama,
		      const char *payload) {
  yama_record *result;
  int datalen = strlen(payload);
  int aligned_len = (datalen & ~7) + 8;
  int newsize = yama->payload->size + sizeof(yama_record) + aligned_len;
  yama->payload = realloc(yama->payload, newsize);
  char *_result = yama->payload->payload + yama->payload->size;
  result = (yama_record *) _result;
  result->size = datalen;
  memcpy(result->payload, payload, datalen);
  result->next = yama->first;
  yama->first = yama->payload->size;
  yama->payload->size = newsize;
  return result;
}
