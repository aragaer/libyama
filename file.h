#ifndef _FILE_H_
#define _FILE_H_
#include "list.h"
#pragma GCC visibility push(internal)

typedef struct yama_payload_s *yama_payload;

yama_payload yama_map(int fd);
void yama_unmap(yama_payload payload);
yama_payload yama_grow(yama_payload payload, int fd, int len);
list_head *records(yama_payload payload);
size_t tail(yama_payload payload);

#pragma GCC visibility pop
#endif  // _FILE_H_
