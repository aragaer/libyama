#ifndef _YAMA_H_

typedef struct {
} yama_record;

typedef struct {
  char magic[4];
} yama_header;

yama_header *yama_new();
void *yama_first(const yama_header *);
yama_record *yama_add(const yama_header *, const char *);

#endif  // _YAMA_H_
