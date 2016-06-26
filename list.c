#include "list.h"

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
