#ifndef _YAMA_H_
#define _YAMA_H_

#include <stdint.h>
#include <string.h>
#include "list.h"

typedef struct yama_s YAMA;
typedef struct yama_item_s yama_item;

YAMA *yama_read(int fd);
void yama_release(YAMA *);

yama_item *yama_first(YAMA *yama);
yama_item *yama_next(yama_item *item);
yama_item *yama_before(yama_item *item, yama_item *history);

yama_item *yama_add(YAMA *yama, char *data, size_t len);
yama_item *yama_insert_after(YAMA *yama, yama_item *prev, char *data, size_t len);
yama_item *yama_edit(YAMA *yama, yama_item *old, char *data, size_t len);
void yama_mark_done(YAMA *yama, yama_item *item);

#define yama_add_string(yama, string) ({	\
      char *_data = (string);			\
      yama_add(yama, _data, strlen(_data)); })

#define yama_insert_string_after(yama, prev, string) ({		\
      char *_data = (string);					\
      yama_insert_after(yama, prev, _data, strlen(_data)); })

#define yama_edit_string(yama, old, string) ({		\
      char *_data = (string);				\
      yama_edit(yama, old, _data, strlen(_data)); })

int size(yama_item *item);
const char *payload(yama_item *item);

#endif  // _YAMA_H_
