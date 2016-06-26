#include "list.h"

void list_init(list_item * const item) {
  item->next = item->previous = 0;
}

list_item *next(list_item const * const item) {
  return (list_item *) ((char *) item + item->next);
}

list_item *prev(list_item const * const item) {
  return (list_item *) ((char *) item + item->previous);
}

static char *get_next(list_item const * const item) {
  return (char *) item + item->next;
}

static void set_next(list_item * const item, const char *value) {
  item->next = value - (char *) item;
}

static char *get_prev(list_item const * const item) {
  return (char *) item + item->previous;
}

static void set_prev(list_item * const item, const char *value) {
  item->previous = value - (char *) item;
}

void list_remove(list_item const * const item) {
  set_prev(next(item), get_prev(item));
  set_next(prev(item), get_next(item));
}

void list_insert_after(list_item * const new, list_item * const item) {
  set_next(new, get_next(item));
  set_prev(new, (char *) item);
  set_prev(next(item), (char *) new);
  set_next(item, (char *) new);
}
