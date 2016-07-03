#ifndef _LIST_H_
#define _LIST_H_
#include <stdint.h>

#pragma GCC visibility push(internal)

typedef struct __attribute__((packed, aligned(4))) {
  int32_t previous, next;
} list_item;

list_item *next(list_item const * const);
list_item *prev(list_item const * const);

void list_init(list_item * const);
void list_remove(list_item const * const);
void list_insert_after(list_item * const, list_item * const);

#define container_of(ptr, type, member) ((type *) ((char *) (ptr)	\
						   - offsetof(type, member)))

#pragma GCC visibility pop

#endif  // _LIST_H_
