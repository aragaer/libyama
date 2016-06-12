#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "yama.h"

struct __attribute__((packed, aligned(4))) yama_header {
  char magic[4];
  int32_t size;
  uint32_t first;
};

struct __attribute__((packed, aligned(4))) yama_payload {
  struct yama_header header;
  char payload[0];
};

static char _magic[] = {'Y', 'A', 'M', 'A'};

static inline int round_up_to(uint32_t value, int round) {
  return (value & ~(round - 1)) + round;
}

static struct yama_payload *new_payload() {
  struct yama_payload *result;
  int size = sizeof(struct yama_payload);
  result = malloc(size);
  memset(result, 0, size);
  result->header.size = size;
  result->header.first = -1;
  memcpy(result->header.magic, _magic, sizeof(_magic));
  return result;
}

static YAMA *yama_alloc() {
  YAMA *result = malloc(sizeof(YAMA));
  memset(result, 0, sizeof(*result));
  return result;
}

YAMA *yama_new() {
  YAMA *result = yama_alloc();
  result->payload = new_payload();
  return result;
}

YAMA *yama_read(int fd) {
  YAMA *result = yama_alloc();
  result->payload = new_payload();
  write(fd, result->payload, result->payload->header.size);
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
  char *ptr = (char *) yama->payload + offt;
  return (yama_record *) ptr;
}

yama_record *yama_first(const YAMA *yama) {
  return offt_to_record(yama, yama->payload->header.first);
}

yama_record *yama_next(const YAMA *yama,
		       yama_record *item) {
  return offt_to_record(yama, item->next);
}

yama_record *yama_add(YAMA * const yama,
		      const char *payload) {
  int datalen = strlen(payload);
  int aligned_len = round_up_to(sizeof(yama_record) + datalen, 4);
  uint32_t oldsize = yama->payload->header.size;
  int newsize = oldsize + aligned_len;

  yama->payload = realloc(yama->payload, newsize);
  yama_record *result = offt_to_record(yama, oldsize);
  result->size = datalen;
  result->next = yama->payload->header.first;
  memcpy(result->payload, payload, datalen);
  yama->payload->header.first = oldsize;
  yama->payload->header.size = newsize;
  return result;
}
