#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include "yama.h"

static void read_yama(YAMA *yama) {
  yama_item *item;
  for (item = yama_latest(yama); item; item = yama_previous(item))
    printf("%.*s\n", size(item), payload(item));
}

static void list_yama(YAMA *yama) {
  yama_item *item;
  for (item = yama_latest(yama); item; item = yama_previous(item)) {
    char buf[80];
    time_t t = timestamp(item);
    strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M", gmtime(&t));
    printf("%08x\t%s\t%.*s\n", id(item), buf, size(item), payload(item));
  }
}

int main(int argc, char *argv[]) {
  int fd = open(argv[2], O_RDWR | O_CREAT, S_IWUSR | S_IRUSR);
  if (fd == -1) {
    perror("open");
    exit(EXIT_FAILURE);
  }

  YAMA *yama = yama_read(fd);

  if (strcmp(argv[1], "read") == 0)
    read_yama(yama);
  else if (strcmp(argv[1], "write") == 0)
    yama_add_string(yama, argv[3]);
  else if (strcmp(argv[1], "list") == 0)
    list_yama(yama);

  yama_release(yama);
  close(fd);
  return 0;
}
