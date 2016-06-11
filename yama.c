#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "yama.h"

struct __attribute__((packed, aligned(4))) yama_header {
  char magic[4];
  int32_t size;
};

struct __attribute__((packed, aligned(4))) yama_payload {
  struct yama_header header;
  char payload[0];
};

static char _magic[4] = {'Y', 'A', 'M', 'A'};

/*
static void yama_init(YAMA *yama, yama_payload *payload) {
  memset(result, 0, sizeof(*result));
  memcpy(result, _magic, sizeof(_magic));
  result->payload = malloc(sizeof(yama_payload));
  memset(result->payload, 0, sizeof(yama_payload));
  result->first = -1;
}
*/

static inline int round_up_to(uint32_t value, int round) {
  return (value & ~(round - 1)) + round;
}

static struct yama_payload *new_payload() {
  struct yama_payload *result;
  int size = sizeof(struct yama_payload);
  result = malloc(size);
  memset(result, 0, size);
  result->header.size = size;
  memcpy(result->header.magic, _magic, sizeof(_magic));
  return result;
}

YAMA *yama_new() {
  YAMA *result = malloc(sizeof(YAMA));
  memset(result, 0, sizeof(*result));
  memcpy(result, _magic, sizeof(_magic));
  result->payload = new_payload();
  result->first = -1;
  return result;
}

YAMA *yama_read(int fd) {
  YAMA *result = yama_new();
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
  int aligned_len = round_up_to(sizeof(yama_record) + datalen, 8);
  int newsize = yama->payload->header.size + aligned_len;
  yama->payload = realloc(yama->payload, newsize);
  uint32_t old_end = yama->payload->header.size;
  char *_result = (char *) yama->payload + old_end;
  result = (yama_record *) _result;
  result->size = datalen;
  memcpy(result->payload, payload, datalen);
  result->next = yama->first;
  yama->first = old_end;
  yama->payload->header.size = newsize;
  return result;
}
