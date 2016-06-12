#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "yama.h"

static void read_yama(YAMA *yama) {
  yama_record *item;
  for (item = yama_first(yama); item; item = yama_next(yama, item)) {
    write(1, item->payload, item->size);
    write(1, "\n", 1);
  }
}

int main(int argc, char *argv[]) {
  int fd = open(argv[2], O_RDWR | O_CREAT, S_IWUSR | S_IRUSR);
  if (fd == -1) {
    perror("open");
    exit(-1);
  }

  YAMA *yama = yama_read(fd);

  if (strcmp(argv[1], "read") == 0)
    read_yama(yama);
  else if (strcmp(argv[1], "write") == 0)
    yama_add(yama, argv[3]);

  yama_release(yama);
  close(fd);
  return 0;
}
