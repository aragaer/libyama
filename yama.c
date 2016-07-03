#define _GNU_SOURCE
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <unistd.h>
#include "list.h"
#include "yama.h"

// #define DEBUG

#ifdef DEBUG
#define DPRINTF(...) do { \
    fprintf(stderr, __VA_ARGS__); \
  } while(0);
#else
#define DPRINTF(...)
#endif

#define ALIGN 4
struct __attribute__((packed, aligned(ALIGN))) yama_record_s {
  int32_t size;
  list_item list;
  list_item log;
  char payload[0];
};

static char _magic[] = {'Y', 'A', 'M', 'A'};

struct __attribute__((packed, aligned(ALIGN))) yama_header {
  char magic[sizeof(_magic)];
  int32_t size;
  list_item sentinel;
};

struct __attribute__((packed, aligned(ALIGN))) yama_payload {
  struct yama_header header;
  char payload[0];
};

#define ROUND_UP(size) (((size) + ALIGN - 1) & ~(ALIGN - 1))

static void init_payload(struct yama_payload *payload) {
  int size = sizeof(struct yama_payload);
  memset(payload, 0, size);
  payload->header.size = size;
  list_init(&payload->header.sentinel);
  memcpy(payload->header.magic, _magic, sizeof(_magic));
}

YAMA *yama_new() {
  char template[] = ".yamaXXXXXX";
  int fd = mkstemp(template);
  if (fd == -1)
    perror("mkstemp");
  else
    unlink(template);
  return yama_read(fd);
}

YAMA *yama_read(int fd) {
  YAMA *result = calloc(1, sizeof(YAMA));
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
  munmap(yama->payload, yama->payload->header.size);
  free(yama);
}

static inline list_item *sentinel(YAMA const * const yama) {
  return &yama->payload->header.sentinel;
}

static inline yama_record *list_item_to_record(YAMA const * const yama,
					       list_item const * const item) {
  return item == sentinel(yama)
    ? NULL
    : container_of(item, yama_record, list);
}

yama_record *yama_first(const YAMA *yama) {
  return list_item_to_record(yama, next(sentinel(yama)));
}

yama_record *yama_next(const YAMA *yama, yama_record *item) {
  return list_item_to_record(yama, next(&item->list));
}

yama_record *yama_prev(const YAMA *yama, yama_record *item) {
  return list_item_to_record(yama, prev(&item->list));
}

int size(yama_record const * const item) {
  return item->size;
}

const char *payload(yama_record const * const item) {
  return item->payload;
}

static void yama_resize(YAMA * const yama, int newsize) {
  yama->payload = mremap(yama->payload, yama->payload->header.size,
			 newsize, MREMAP_MAYMOVE);
  if (ftruncate(yama->fd, newsize) == -1)
    perror("ftruncate");
  yama->payload->header.size = newsize;
}

static yama_record *_store(YAMA * const yama, char const *payload) {
  int datalen = strlen(payload);
  int aligned_len = ROUND_UP(sizeof(yama_record) + datalen);
  uint32_t oldsize = yama->payload->header.size;
  int newsize = oldsize + aligned_len;
  yama_resize(yama, newsize);
  yama_record *result = (yama_record *) ((char *) yama->payload + oldsize);
  result->size = datalen;
  memset(&result->list, 0, sizeof(list_item));
  memset(&result->log, 0, sizeof(list_item));
  memcpy(result->payload, payload, datalen);
  return result;
}

yama_record *yama_add(YAMA * const yama, const char *payload) {
  yama_record *result = _store(yama, payload);
  list_insert_after(&result->list, sentinel(yama));
  return result;
}

yama_record *yama_insert_after(YAMA * const yama,
			       yama_record *item,
			       char const *payload) {
  yama_record *result = _store(yama, payload);
  list_insert_after(&result->list, &item->list);
  return result;
}

yama_record *yama_edit(YAMA * const yama,
		       yama_record *item,
		       char const *payload) {
  yama_record *result = _store(yama, payload);
  list_remove(&item->list);
  list_insert_after(&result->list, prev(&item->list));
  list_insert_after(&result->log, prev(&item->log));
  return result;
}

yama_record *yama_before(yama_record const * const item,
			 yama_record const * const history) {
  list_item *log_next = next(&history->log);
  if (log_next == &item->log)
    return NULL;
  return container_of(log_next, yama_record, log);
}
