#define _GNU_SOURCE
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
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

struct yama_s {
  int fd;
  list_head *records;
  struct yama_payload *payload;
  yama_item *first, *first_item, *last_item;
};

#define ALIGN 4
struct __attribute__((packed, aligned(ALIGN))) yama_record_s {
  int32_t size;
  list_head list;
  list_head log;
  char payload[0];
};

struct yama_item_s {
  size_t record;
  struct yama_item_s *next;
  YAMA *yama;
};

static char _magic[] = {'Y', 'A', 'M', 'A'};

struct __attribute__((packed, aligned(ALIGN))) yama_header {
  char magic[sizeof(_magic)];
  int32_t size;
  list_head records;
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
  list_init_head(&payload->header.records);
  memcpy(payload->header.magic, _magic, sizeof(_magic));
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
  result->records = &result->payload->header.records;
  return result;
}

void yama_release(YAMA *yama) {
  munmap(yama->payload, yama->payload->header.size);
  yama_item *item = yama->first_item;
  while (item) {
    yama_item *to_free = item;
    item = item->next;
    free(to_free);
  }
  free(yama);
}

static inline yama_record *list_item_to_record(list_head const * const item) {
  return item == NULL
    ? NULL
    : container_of(item, yama_record, list);
}

yama_record *yama_next(const YAMA *yama, yama_record *item) {
  return list_item_to_record(list_get_next(&item->list, yama->records));
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
  yama->records = &yama->payload->header.records;
}

static yama_record *_yama_store(YAMA * const yama, char const *payload, size_t datalen) {
  int aligned_len = ROUND_UP(sizeof(yama_record) + datalen);
  uint32_t oldsize = yama->payload->header.size;
  int newsize = oldsize + aligned_len;
  yama_resize(yama, newsize);
  yama_record *result = (yama_record *) ((char *) yama->payload + oldsize);
  result->size = datalen;
  list_init_head(&result->list);
  list_init_head(&result->log);
  memcpy(result->payload, payload, datalen);
  return result;
}

yama_record *yama_before(yama_record *item,
			 yama_record *history) {
  list_head *log_next = list_get_next(&item->log, &history->log);
  return log_next == NULL
    ? NULL
    : container_of(log_next, yama_record, log);
}

yama_record *yama_add(YAMA *yama, char *data, size_t len) {
  yama_record *result = _yama_store(yama, data, len);
  list_insert(&result->list, yama->records);
  return result;
}

yama_record *yama_insert_after(YAMA *yama, yama_record *prev,
			       char *data, size_t len) {
  yama_record *result = _yama_store(yama, data, len);
  list_insert(&result->list, &prev->list);
  return result;
}

yama_record *yama_edit(YAMA *yama, yama_record *old, char *data, size_t len) {
  yama_record *result = _yama_store(yama, data, len);
  list_replace(&result->list, &old->list);
  list_add_tail(&result->log, &old->log);
  return result;
}

void yama_mark_done(YAMA *yama, yama_record *item) {
  list_remove(&item->list);
}

static inline yama_record *item2record(yama_item *item) {
  return (yama_record *) ((char *) item->yama->payload + item->record);
}

yama_item *record2item(YAMA *yama, yama_record *record) {
  if (record == NULL)
    return NULL;
  size_t record_offt = (char *) record - (char *) yama->payload;
  yama_item *item;
  for (item = yama->first_item; item; item = item->next)
    if (item->record == record_offt)
      return item;
  item = calloc(1, sizeof(*item));
  item->yama = yama;
  item->record = (char *) record - (char *) yama->payload;
  if (yama->last_item != NULL)
    yama->last_item->next = item;
  if (yama->first_item == NULL)
    yama->first_item = item;
  return item;
}

yama_item *yama_first_item(YAMA *yama) {
  list_head *first_head = list_get_next(yama->records, yama->records);
  yama_record *first_record = list_item_to_record(first_head);
  return record2item(yama, first_record);
}

int item_size(yama_item *item) {
  return size(item2record(item));
}

const char *item_payload(yama_item *item) {
  return payload(item2record(item));
}

yama_item *yama_next_item(yama_item *item) {
  yama_record *record = yama_next(item->yama, item2record(item));
  return record2item(item->yama, record);
}

yama_record *get_record(yama_item *item) {
  return item2record(item);
}
