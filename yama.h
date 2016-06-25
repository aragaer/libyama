#ifndef _YAMA_H_

#include <stdint.h>

typedef struct {
  int32_t size;
  int32_t next;
  int32_t previous;
  char payload[0];
} yama_record;

typedef struct {
  int fd;
  struct yama_payload *payload;
} YAMA;

YAMA *yama_new();
YAMA *yama_read(int fd);
void yama_release(YAMA *);
yama_record *yama_first(YAMA const * const);
yama_record *yama_next(YAMA const * const, yama_record *);
yama_record *yama_prev(YAMA const * const, yama_record *);
yama_record *yama_add(YAMA * const, char const * const);
yama_record *yama_insert_after(YAMA * const,
			       yama_record *,
			       char const * const);
yama_record *yama_edit(YAMA * const, yama_record *,
		       char const * const);

#endif  // _YAMA_H_
