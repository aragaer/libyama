#include <stddef.h>
#include "list.h"

void list_init_head(list_head *head) {
  head->next = head->prev = 0;
}

int list_is_empty(list_head const * const head) {
  return head->next == 0;
}

#define GET_NEXT(item) ({list_head *_p = (item);	\
      (list_head *) ((char *) _p + _p->next); })

#define GET_PREV(item) ({list_head *_p = (item);	\
      (list_head *) ((char *) _p + _p->prev); })

#define SET_NEXT(item1, item2) do {				\
    list_head *_p = (item1);					\
    _p->next = (int64_t) ((char *) (item2) - (char *) _p);	\
  } while (0);

#define SET_PREV(item1, item2) do {				\
    list_head *_p = (item1);					\
    _p->prev = (int64_t) ((char *) (item2) - (char *) _p);	\
  } while (0);

void list_add(list_head *new, list_head *head) {
  SET_NEXT(new, GET_NEXT(head));
  SET_PREV(new, head);
  SET_NEXT(head, new);
  SET_PREV(GET_NEXT(new), new);
}

void list_insert(list_head *new, list_head *prev) {
  list_add(new, prev);
}

void list_replace(list_head *new, list_head *old) {
  SET_NEXT(new, GET_NEXT(old));
  SET_NEXT(GET_PREV(old), new);
}

void list_remove(list_head *item) {
  SET_NEXT(GET_PREV(item), GET_NEXT(item));
}

list_head *list_get_next(list_head *item, list_head *head) {
  list_head *result = GET_NEXT(item);
  return result == head ? NULL : result;
}
