#ifndef _FILE_H_
#define _FILE_H_
#include "list.h"
#pragma GCC visibility push(internal)

typedef struct yama_file_s *yama_file;

yama_file yama_map(int fd);
void yama_unmap(yama_file file);
yama_file yama_grow(yama_file file, int fd, int len);
list_head *records(yama_file file);
size_t tail(yama_file file);

#pragma GCC visibility pop
#endif  // _FILE_H_
