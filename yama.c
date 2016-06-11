#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "yama.h"

struct __attribute__((packed, aligned(4))) yama_payload {
  char magic[4];
  int32_t size;
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

#include <stdio.h>
static struct yama_payload *new_payload() {
  struct yama_payload *result;
  int size = sizeof(struct yama_payload);
  result = malloc(size);
  memset(result, 0, size);
  result->size = size;
  fprintf(stderr, "Payload at %p, size %zu\n",
	  result, result->size);
  memcpy(result->magic, _magic, sizeof(_magic));
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
  write(fd, result->payload, result->payload->size);
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

static void dump_payload(struct yama_payload *payload) {
  fprintf(stderr, "XXX: Size = %u\n", payload->size);
  char *ptr = payload->payload;
  yama_record *record = (yama_record *) ptr;
  fprintf(stderr, "XXX: Item at %p\n", record);
  fprintf(stderr, "XXX: item size = %u\n", record->size);
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
  fprintf(stderr, "Data is [%s], increasing by %d (%d)\n",
	  payload, datalen, aligned_len);
  int newsize = yama->payload->size + aligned_len;
  yama->payload = realloc(yama->payload, newsize);
  fprintf(stderr, "Payload at %p, size %zu\n",
	  yama->payload, newsize);
  char *_result = (char *) yama->payload + yama->payload->size;
  result = (yama_record *) _result;
  result->size = datalen;
  memcpy(result->payload, payload, datalen);
  result->next = yama->first;
  yama->first = yama->payload->size;
  yama->payload->size = newsize;
  dump_payload(yama->payload);
  return result;
}
