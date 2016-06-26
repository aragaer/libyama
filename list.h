#include <stdint.h>

typedef struct list_item_s list_item; 

struct __attribute__((packed, aligned(4))) list_item_s {
  int32_t next;
  int32_t previous;
};

int32_t get_next(list_item const * const);
void set_next(list_item * const, const int32_t);
int32_t get_prev(list_item const * const);
void set_prev(list_item * const, const int32_t);
