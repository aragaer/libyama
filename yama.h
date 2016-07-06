#ifndef _YAMA_H_
#define _YAMA_H_

#include <stdint.h>
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
yama_record *yama_add(YAMA * const, char const * const);
yama_record *yama_insert_after(YAMA * const,
			       yama_record *,
			       char const * const);
yama_record *yama_edit(YAMA * const, yama_record *,
		       char const * const);

yama_record *yama_add_binary(YAMA *yama, char *data, size_t len);

yama_record *yama_before(yama_record *item, yama_record *history);

int size(yama_record const * const);
const char *payload(yama_record const * const);

#endif  // _YAMA_H_
