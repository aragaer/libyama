#ifndef _YAMA_H_

typedef struct {
  int size;
  void *next;
  char payload[0];
} yama_record;

typedef struct {
  char magic[4];
  yama_record *first;
} YAMA;

YAMA *yama_new();
YAMA *yama_read(int fd);
void yama_release(YAMA *);
yama_record *yama_first(YAMA const * const);
yama_record *yama_next(YAMA const * const, yama_record *);
yama_record *yama_add(YAMA * const, char const * const);

#endif  // _YAMA_H_
