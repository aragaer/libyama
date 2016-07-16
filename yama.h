#ifndef _YAMA_H_
#define _YAMA_H_

#include <stdint.h>
#include <string.h>
#include "list.h"

typedef struct yama_record_s yama_record;

typedef struct {
  int fd;
  list_head *records;
  struct yama_payload *payload;
} YAMA;

YAMA *yama_new();
YAMA *yama_read(int fd);
void yama_release(YAMA *);
yama_record *yama_first(YAMA const * const);
yama_record *yama_next(YAMA const * const, yama_record *);

yama_record *yama_add(YAMA *yama, char *data, size_t len);
yama_record *yama_insert_after(YAMA *yama, yama_record *prev, char *data, size_t len);
yama_record *yama_edit(YAMA *yama, yama_record *old, char *data, size_t len);
void yama_mark_done(YAMA *yama, yama_record *item);

#define yama_add_string(yama, string) ({	\
      char *_data = (string);			\
      yama_add(yama, _data, strlen(_data)); })

#define yama_insert_string_after(yama, prev, string) ({		\
      char *_data = (string);					\
      yama_insert_after(yama, prev, _data, strlen(_data)); })

#define yama_edit_string(yama, old, string) ({		\
      char *_data = (string);				\
      yama_edit(yama, old, _data, strlen(_data)); })


yama_record *yama_before(yama_record *item, yama_record *history);

int size(yama_record const * const);
const char *payload(yama_record const * const);

#endif  // _YAMA_H_
