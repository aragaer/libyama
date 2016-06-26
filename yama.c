#define _GNU_SOURCE
#include <stddef.h>
#include <stdio.h>
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
  yama_record sentinel;
};

struct __attribute__((packed, aligned(4))) yama_payload {
  struct yama_header header;
  char payload[0];
};

#define NO_RECORD ((int32_t) offsetof(struct yama_payload, header.sentinel))

static char _magic[] = {'Y', 'A', 'M', 'A'};

static inline int32_t round_up_to(uint32_t value, const int round) {
  return (value & ~(round - 1)) + round;
}

static inline yama_record *offt_to_record(const YAMA *yama,
					  const int32_t offt) {
  char *ptr = (char *) yama->payload + offt;
  return (yama_record *) ptr;
}

static inline int32_t record_to_offt(const YAMA *yama,
				     const yama_record *item) {
  size_t diff = (char *) item - (char *) yama->payload;
  return (int32_t) diff;
}


static struct yama_payload *alloc_payload() {
  return malloc(sizeof(struct yama_payload));
}

static void init_payload(struct yama_payload *payload) {
  int size = sizeof(struct yama_payload);
  memset(payload, 0, size);
  payload->header.size = size;
  payload->header.sentinel.next =
    payload->header.sentinel.previous = offsetof(struct yama_payload,
						 header.sentinel);
  payload->header.sentinel.size = sizeof(yama_record);
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
    if (ftruncate(fd, sizeof(header)) == -1)
      perror("ftruncate");
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

static inline yama_record *sentinel(const YAMA *yama) {
  return &yama->payload->header.sentinel;
}

static inline yama_record *_next(const YAMA *yama, yama_record *item) {
  return offt_to_record(yama, item->next);
}

static inline yama_record *_prev(const YAMA *yama, yama_record *item) {
  return offt_to_record(yama, item->previous);
}

static inline void _remove(const YAMA *yama, yama_record *item) {
  _next(yama, item)->previous = item->previous;
  _prev(yama, item)->next = item->next;
}

static inline void _insert_after(const YAMA *yama,
				 yama_record *item,
				 yama_record *new) {
  new->next = item->next;
  new->previous = record_to_offt(yama, item);
  _next(yama, item)->previous = record_to_offt(yama, new);
  item->next = record_to_offt(yama, new);
}

static inline yama_record *sentinel_to_null(const YAMA *yama,
					    yama_record *item) {
  return item == sentinel(yama) ? NULL : item;
}

yama_record *yama_first(const YAMA *yama) {
  yama_record *result = _next(yama, sentinel(yama));
  return sentinel_to_null(yama, result);
}

yama_record *yama_next(const YAMA *yama, yama_record *item) {
  yama_record *result = _next(yama, item);
  return sentinel_to_null(yama, result);
}

yama_record *yama_prev(const YAMA *yama, yama_record *item) {
  yama_record *result = _prev(yama, item);
  return sentinel_to_null(yama, result);
}

static void yama_resize(YAMA * const yama, int newsize) {
  if (yama->fd == -1)
    yama->payload = realloc(yama->payload, newsize);
  else {
    yama->payload = mremap(yama->payload, yama->payload->header.size,
			   newsize, MREMAP_MAYMOVE);
    if (ftruncate(yama->fd, newsize) == -1)
      perror("ftruncate");
  }
  yama->payload->header.size = newsize;
}

static yama_record *_store(YAMA * const yama,
			       char const *payload) {
  int datalen = strlen(payload);
  int aligned_len = round_up_to(sizeof(yama_record) + datalen, 4);
  uint32_t oldsize = yama->payload->header.size;
  int newsize = oldsize + aligned_len;
  yama_resize(yama, newsize);
  yama_record *result = offt_to_record(yama, oldsize);
  result->size = datalen;
  result->next = NO_RECORD;
  result->previous = NO_RECORD;
  memcpy(result->payload, payload, datalen);
  return result;
}

yama_record *yama_add(YAMA * const yama,
		      const char *payload) {
  yama_record *result = _store(yama, payload);
  _insert_after(yama, sentinel(yama), result);
  return result;
}

yama_record *yama_insert_after(YAMA * const yama,
			       yama_record *item,
			       char const *payload) {
  yama_record *result = _store(yama, payload);
  _insert_after(yama, item, result);
  return result;
}

yama_record *yama_edit(YAMA * const yama,
		       yama_record *item,
		       char const *payload) {
  yama_record *result = _store(yama, payload);
  _remove(yama, item);
  _insert_after(yama, _prev(yama, item), result);
  return result;
}
