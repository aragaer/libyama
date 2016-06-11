#ifndef _YAMA_H_

typedef struct {
  int size;
  char payload[0];
} yama_record;

typedef struct {
  char magic[4];
  yama_record *first;
} yama_header;

yama_header *yama_new();
void *yama_first(yama_header const * const);
yama_record *yama_add(yama_header * const,
		      char const * const);

#endif  // _YAMA_H_
