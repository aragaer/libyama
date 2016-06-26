#include <stdint.h>

typedef struct list_item_s list_item; 

struct __attribute__((packed, aligned(4))) list_item_s {
  int32_t next;
  int32_t previous;
};

list_item *next(list_item const * const);
list_item *prev(list_item const * const);

void list_remove(list_item const * const);
void list_insert_after(list_item * const, list_item * const);

int32_t get_next(list_item const * const);
void set_next(list_item * const, const int32_t);
int32_t get_prev(list_item const * const);
void set_prev(list_item * const, const int32_t);

#define container_of(ptr, type, member) ({                      \
      const typeof( ((type *)0)->member ) *__mptr = (ptr);	\
      (type *)( (char *)__mptr - offsetof(type,member) );})
