#ifndef _YAMA_H_

struct yama_record_s {
  int size;
  struct yama_record_s *next;
  char payload[0];
};

typedef struct yama_record_s yama_record;

typedef struct {
  char magic[4];
  yama_record *first;
} yama_header;

yama_header *yama_new();
void *yama_first(yama_header const * const);
yama_record *yama_add(yama_header * const,
		      char const * const);

#endif  // _YAMA_H_
