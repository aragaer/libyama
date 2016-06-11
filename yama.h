#ifndef _YAMA_H_

typedef struct {
  char magic[4];
} yama_header;

yama_header *yama_new();

#endif  // _YAMA_H_
