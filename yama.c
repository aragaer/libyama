#define _GNU_SOURCE
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <unistd.h>
#include "yama.h"

// #define DEBUG

#ifdef DEBUG
#include <stdio.h>
#define DPRINTF(...) do { \
    fprintf(stderr, __VA_ARGS__); \
  } while(0);
#else
#define DPRINTF(...)
#endif

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

static struct yama_payload *alloc_payload() {
  return malloc(sizeof(struct yama_payload));
}

static void init_payload(struct yama_payload *payload) {
  int size = sizeof(struct yama_payload);
  memset(payload, 0, size);
  payload->header.size = size;
  payload->header.first = -1;
  memcpy(payload->header.magic, _magic, sizeof(_magic));
}

static struct yama_payload *new_payload() {
  struct yama_payload *result = alloc_payload();
  init_payload(result);
  return result;
}

static YAMA *yama_alloc() {
  YAMA *result = malloc(sizeof(YAMA));
  memset(result, 0, sizeof(*result));
  return result;
}

YAMA *yama_new() {
  YAMA *result = yama_alloc();
  result->fd = -1;
  result->payload = new_payload();
  return result;
}

YAMA *yama_read(int fd) {
  YAMA *result = yama_alloc();
  result->fd = fd;
  struct yama_header header;
  int bytes_read = read(fd, &header, sizeof(header));
  DPRINTF("Bytes read: %d\n", bytes_read);
  if (bytes_read == 0) {
    ftruncate(fd, sizeof(header));
    result->payload = mmap(NULL, sizeof(header), PROT_READ | PROT_WRITE,
			   MAP_SHARED, fd, 0);
    init_payload(result->payload);
  } else {
    result->payload = mmap(NULL, header.size, PROT_READ | PROT_WRITE,
			   MAP_SHARED, fd, 0);
  }
  return result;
}

void yama_release(YAMA *yama) {
  if (yama->fd == -1)
    free(yama->payload);
  else
    munmap(yama->payload, yama->payload->header.size);
  free(yama);
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

yama_record *yama_next(const YAMA *yama, yama_record *item) {
  return offt_to_record(yama, item->next);
}

static void yama_resize(YAMA * const yama, int newsize) {
  if (yama->fd == -1)
    yama->payload = realloc(yama->payload, newsize);
  else {
    yama->payload = mremap(yama->payload, yama->payload->header.size,
			   newsize, MREMAP_MAYMOVE);
    ftruncate(yama->fd, newsize);
  }
  yama->payload->header.size = newsize;
}

yama_record *yama_add(YAMA * const yama,
		      const char *payload) {
  int datalen = strlen(payload);
  int aligned_len = round_up_to(sizeof(yama_record) + datalen, 4);
  uint32_t oldsize = yama->payload->header.size;
  int newsize = oldsize + aligned_len;

  yama_resize(yama, newsize);
  yama_record *result = offt_to_record(yama, oldsize);
  result->size = datalen;
  result->next = yama->payload->header.first;
  memcpy(result->payload, payload, datalen);
  yama->payload->header.first = oldsize;
  return result;
}
