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
  yama_item *first_item, *last_item;
};

struct yama_item_s {
  size_t record;
  struct yama_item_s *next;
  YAMA *yama;
};

#define ALIGN 4
#define PACKED __attribute__((packed, aligned(ALIGN)))

typedef struct PACKED {
  int32_t size;
  list_head list;
  list_head log;
  int done: 1;
  char payload[0];
} yama_record;

static char _magic[] = {'Y', 'A', 'M', 'A'};

typedef struct PACKED {
  char magic[sizeof(_magic)];
  int32_t size;
  list_head records;
} yama_header;

struct PACKED yama_payload {
  yama_header header;
  char payload[0];
};

#define ROUND_UP(size) (((size) + ALIGN - 1) & ~(ALIGN - 1))

static inline yama_record *get_record(yama_item *item) {
  return (yama_record *) ((char *) item->yama->payload + item->record);
}

static inline size_t record2offt(YAMA *yama, yama_record *record) {
  return (char *) record - (char *) yama->payload;
}

static inline yama_item *search_item(YAMA *yama, size_t record_offt) {
  yama_item *item;
  for (item = yama->first_item; item; item = item->next)
    if (item->record == record_offt)
      break;
  return item;
}

static inline yama_item *new_item(YAMA *yama, size_t record_offt) {
  yama_item *item = calloc(1, sizeof(*item));
  item->yama = yama;
  item->record = record_offt;
  if (yama->first_item == NULL)
    yama->first_item = item;
  else
    yama->last_item->next = item;
  yama->last_item = item;
  return item;
}

static inline yama_item *get_item(YAMA *yama, yama_record *record) {
  if (record == NULL)
    return NULL;
  size_t record_offt = record2offt(yama, record);
  yama_item *item = search_item(yama, record_offt);
  if (item == NULL)
    item = new_item(yama, record_offt);
  return item;
}

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
  yama_header header;
  int bytes_read = read(fd, &header, sizeof(header));
  DPRINTF("Bytes read: %d\n", bytes_read);
  if (bytes_read == 0) {
    if (ftruncate(fd, sizeof(header)) == -1)
      perror("ftruncate");
    result->payload = mmap(NULL, sizeof(header), PROT_READ | PROT_WRITE,
			   MAP_SHARED, fd, 0);
    init_payload(result->payload);
  } else
    result->payload = mmap(NULL, header.size, PROT_READ | PROT_WRITE,
			   MAP_SHARED, fd, 0);
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

int size(yama_item *item) {
  return get_record(item)->size;
}

const char *payload(yama_item *item) {
  return get_record(item)->payload;
}

int is_done(yama_item *item) {
  return get_record(item)->done;
}

static void _yama_resize(YAMA *yama, size_t newsize) {
  size_t oldsize = yama->payload->header.size;
  yama->payload = mremap(yama->payload, oldsize, newsize, MREMAP_MAYMOVE);
  if (ftruncate(yama->fd, newsize) == -1)
    perror("ftruncate");
  yama->payload->header.size = newsize;
  yama->records = &yama->payload->header.records;
}

static yama_record *_yama_store(YAMA *yama, char *payload, size_t datalen) {
  int aligned_len = ROUND_UP(sizeof(yama_record) + datalen);
  size_t oldsize = yama->payload->header.size;
  _yama_resize(yama, oldsize + aligned_len);
  yama_record *result = (yama_record *) ((char *) yama->payload + oldsize);
  result->size = datalen;
  list_init_head(&result->list);
  list_init_head(&result->log);
  memcpy(result->payload, payload, datalen);
  return result;
}

yama_item *yama_before(yama_item *item, yama_item *history) {
  yama_record *record = get_record(item);
  yama_record *history_record = get_record(history);
  list_head *log_next = list_get_next(&record->log, &history_record->log);
  return log_next == NULL
    ? NULL
    : get_item(item->yama, container_of(log_next, yama_record, log));
}

yama_item *yama_add(YAMA *yama, char *data, size_t len) {
  yama_record *record = _yama_store(yama, data, len);
  list_insert(&record->list, yama->records);
  return new_item(yama, record2offt(yama, record));
}

yama_item *yama_insert_after(yama_item *prev, char *data, size_t len) {
  YAMA *yama = prev->yama;
  yama_record *result = _yama_store(yama, data, len);
  yama_record *prev_record = get_record(prev);
  list_insert(&result->list, &prev_record->list);
  return new_item(yama, record2offt(yama, result));
}

yama_item *yama_edit(yama_item *old, char *data, size_t len) {
  YAMA *yama = old->yama;
  yama_record *new_record = _yama_store(yama, data, len);
  yama_record *old_record = get_record(old);
  list_replace(&new_record->list, &old_record->list);
  list_add_tail(&new_record->log, &old_record->log);
  return new_item(yama, record2offt(yama, new_record));
}

void yama_mark_done(yama_item *item) {
  get_record(item)->done = 1;
}

yama_item *yama_latest(YAMA *yama) {
  yama_item *result = yama_full_history(yama);
  if (result && get_record(result)->done)
    return NULL;
  return result;
}

yama_item *yama_previous(yama_item *item) {
  list_head *list_next = list_get_next(&get_record(item)->list,
				       item->yama->records);
  return get_item(item->yama, list_item_to_record(list_next));
}

yama_item *yama_full_history(YAMA *yama) {
  list_head *first_head = list_get_next(yama->records, yama->records);
  return get_item(yama, list_item_to_record(first_head));
}
