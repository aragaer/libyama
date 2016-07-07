#ifndef _LIST_H_
#define _LIST_H_
#include <stdint.h>

#pragma GCC visibility push(internal)

typedef struct __attribute__((packed, aligned(4))) {
  int64_t prev, next;
} list_head;

#define LIST_HEAD_INIT() {0, 0}

void list_init_head(list_head *head);

void list_add(list_head *new, list_head *head);
void list_add_tail(list_head *new, list_head *head);
void list_insert(list_head *new, list_head *prev);
void list_replace(list_head *new, list_head *old);
void list_remove(list_head *item);
list_head *list_get_next(list_head *item, list_head *head);

#define container_of(ptr, type, member) ((type *) ((char *) (ptr)	\
						   - offsetof(type, member)))

#pragma GCC visibility pop

#endif  // _LIST_H_
