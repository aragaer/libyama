#include <stdlib.h>
#include <time.h>
#include "file.h"
#include "list.h"
#include "record.h"
#include "yama.h"

struct yama_s {
  int fd;
  void *records, *file;
  yama_item *first_item, *last_item;
};

struct yama_item_s {
  record_id record;
  struct yama_item_s *next;
  YAMA *yama;
};

static inline yama_record *offt2record(YAMA *yama, record_id offt) {
  return yama->file + offt;
}

static inline record_id record2offt(YAMA *yama, yama_record *record) {
  return (void *) record - yama->file;
}

static inline yama_item *search_item(YAMA *yama, record_id record_id) {
  yama_item *item;
  for (item = yama->first_item; item; item = item->next)
    if (item->record == record_id)
      break;
  return item;
}

static inline yama_item *new_item(YAMA *yama, record_id record_id) {
  yama_item *item = calloc(1, sizeof(*item));
  item->yama = yama;
  item->record = record_id;
  if (yama->first_item == NULL)
    yama->first_item = item;
  else
    yama->last_item->next = item;
  yama->last_item = item;
  return item;
}

static inline yama_record *get_record(yama_item *item) {
  return offt2record(item->yama, item->record);
}

static inline yama_item *get_item_by_id(YAMA *yama, record_id record_id) {
  return search_item(yama, record_id)
    ?: new_item(yama, record_id);
}

static inline yama_item *get_item(YAMA *yama, yama_record *record) {
  return record == NULL
    ? NULL
    : get_item_by_id(yama, record2offt(yama, record));
}

static void yama_bind(YAMA *yama, void *file) {
  yama->file = file;
  yama->records = records(file);
}

YAMA *yama_read(int fd) {
  YAMA *result = calloc(1, sizeof(YAMA));
  result->fd = fd;
  yama_bind(result, yama_map(fd));
  return result;
}

void yama_release(YAMA *yama) {
  yama_unmap(yama->file);
  yama_item *item = yama->first_item;
  while (item) {
    yama_item *to_free = item;
    item = item->next;
    free(to_free);
  }
  free(yama);
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

yama_item *yama_before(yama_item *item, yama_item *history) {
  yama_record *record = get_record(item);
  list_head *log_next = list_get_next(&record->log);
  return log_next == &get_record(history)->log
    ? NULL
    : get_item(item->yama, container_of(log_next, yama_record, log));
}

yama_record *yama_alloc_record(YAMA *yama, size_t len) {
  record_id new_record_id = tail(yama->file);
  void *new_file = yama_grow(yama->file, yama->fd, sizeof(yama_record)+len);
  yama_bind(yama, new_file);
  return offt2record(yama, new_record_id);
}

yama_record *yama_store(YAMA *yama, char *payload, size_t datalen) {
  yama_record *result = yama_alloc_record(yama, datalen);
  yama_init_record(result, payload, datalen);
  return result;
}

yama_item *yama_add(YAMA *yama, char *data, size_t len) {
  yama_record *record = yama_store(yama, data, len);
  list_insert(&record->list, yama->records);
  return new_item(yama, record2offt(yama, record));
}

yama_item *yama_insert_after(yama_item *prev, char *data, size_t len) {
  YAMA *yama = prev->yama;
  yama_record *result = yama_store(yama, data, len);
  yama_record *prev_record = get_record(prev);
  list_insert(&result->list, &prev_record->list);
  return new_item(yama, record2offt(yama, result));
}

yama_item *yama_edit(yama_item *old, char *data, size_t len) {
  YAMA *yama = old->yama;
  yama_record *new_record = yama_store(yama, data, len);
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
  list_head *list_next = list_get_next(&get_record(item)->list);
  return list_next == item->yama->records
    ? NULL
    : get_item(item->yama, list_item_to_record(list_next));
}

yama_item *yama_full_history(YAMA *yama) {
  list_head *first_head = list_get_next(yama->records);
  return first_head == yama->records
    ? NULL
    : get_item(yama, list_item_to_record(first_head));
}

time_t timestamp(yama_item *item) {
  return get_record(item)->timestamp;
}

record_id id(yama_item *item) {
  return item->record;
}

yama_item *yama_get_by_id(YAMA *yama, record_id id) {
  return get_item_by_id(yama, id);
}
