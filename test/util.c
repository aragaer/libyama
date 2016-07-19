#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <unistd.h>
#include <yama.h>

struct yama_s {
  int fd;
  void *records;
  void *file;
};

YAMA *yama_new() {
  char template[] = ".yamaXXXXXX";
  int fd = mkstemp(template);
  if (fd == -1)
    perror("mkstemp");
  else
    unlink(template);
  return yama_read(fd);
}

void test_relocate(YAMA *yama) {
  int size = lseek(yama->fd, 0, SEEK_END);

  void *to_unmap = yama->file;
  yama->file = mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_SHARED, yama->fd, 0);
  yama->records += (void *) yama->file - to_unmap;
  munmap(to_unmap, size);
}
