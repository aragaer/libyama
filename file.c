#define _GNU_SOURCE
#include <stdio.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <unistd.h>
#include "file.h"
#include "yama.h"

#define ROUND_UP(size) (((size) + ALIGN - 1) & ~(ALIGN - 1))

static char _magic[] = {'Y', 'A', 'M', 'A'};

struct PACKED yama_file_s {
  char magic[sizeof(_magic)];
  int32_t size;
  list_head records;
  char file[0];
};

static void init_empty_file(yama_file file) {
  memset(file, 0, sizeof(*file));
  file->size = sizeof(*file);
  list_init_head(&file->records);
  memcpy(file->magic, _magic, sizeof(_magic));
}

yama_file yama_map(int fd) {
  struct yama_file_s header;
  int bytes_read = read(fd, &header, sizeof(header));
  yama_file result;
  if (bytes_read == 0) {
    if (ftruncate(fd, sizeof(header)) == -1)
      perror("ftruncate");
    result = mmap(NULL, sizeof(header), PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    init_empty_file(result);
  } else
    result = mmap(NULL, header.size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
  return result;
}

void yama_unmap(yama_file file) {
  munmap(file, file->size);
}

yama_file yama_grow(yama_file file, int fd, int len) {
  size_t newsize = file->size + ROUND_UP(len);
  yama_file result = mremap(file, file->size, newsize, MREMAP_MAYMOVE);
  if (ftruncate(fd, newsize) == -1)
    perror("ftruncate");
  result->size = newsize;
  return result;
}

list_head *records(yama_file file) {
  return &file->records;
}

size_t tail(yama_file file) {
  return file->size;
}
