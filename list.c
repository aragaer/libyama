#include "list.h"

list_item *next(list_item const * const item) {
  return (list_item *) ((int32_t) item + item->next);
}

list_item *prev(list_item const * const item) {
  return (list_item *) ((int32_t) item + item->previous);
}

int32_t get_next(list_item const * const item) {
  return (int32_t) item + item->next;
}

void set_next(list_item * const item, const int32_t value) {
  item->next = value - (int32_t) item;
}

int32_t get_prev(list_item const * const item) {
  return (int32_t) item + item->previous;
}

void set_prev(list_item * const item, const int32_t value) {
  item->previous = value - (int32_t) item;
}

void list_remove(list_item const * const item) {
  set_prev(next(item), get_prev(item));
  set_next(prev(item), get_next(item));
}

void list_insert_after(list_item * const new, list_item * const item) {
  set_next(new, get_next(item));
  set_prev(new, (int32_t) item);
  set_prev(next(item), (int32_t) new);
  set_next(item, (int32_t) new);
}
