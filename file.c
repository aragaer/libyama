#define _GNU_SOURCE
#include <stdio.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <unistd.h>
#include "file.h"
#include "yama.h"

#define ROUND_UP(size) (((size) + ALIGN - 1) & ~(ALIGN - 1))

static char _magic[] = {'Y', 'A', 'M', 'A'};

typedef struct PACKED {
  char magic[sizeof(_magic)];
  int32_t size;
  list_head records;
} yama_header;

struct PACKED yama_payload_s {
  yama_header header;
  char payload[0];
};

static void init_payload(yama_payload payload) {
  int size = sizeof(*payload);
  memset(payload, 0, size);
  payload->header.size = size;
  list_init_head(&payload->header.records);
  memcpy(payload->header.magic, _magic, sizeof(_magic));
}

yama_payload yama_map(int fd) {
  yama_header header;
  int bytes_read = read(fd, &header, sizeof(header));
  yama_payload result;
  if (bytes_read == 0) {
    if (ftruncate(fd, sizeof(header)) == -1)
      perror("ftruncate");
    result = mmap(NULL, sizeof(header), PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    init_payload(result);
  } else
    result = mmap(NULL, header.size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
  return result;
}

void yama_unmap(yama_payload payload) {
  munmap(payload, payload->header.size);
}

yama_payload yama_grow(yama_payload payload, int fd, int len) {
  int aligned_len = ROUND_UP(len);
  size_t oldsize = payload->header.size;
  size_t newsize = oldsize + aligned_len;
  yama_payload result = mremap(payload, oldsize, newsize, MREMAP_MAYMOVE);
  if (ftruncate(fd, newsize) == -1)
    perror("ftruncate");
  result->header.size = newsize;
  return result;
}

list_head *records(yama_payload payload) {
  return &payload->header.records;
}

size_t tail(yama_payload payload) {
  return payload->header.size;
}
