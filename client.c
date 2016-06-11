#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>
#include "yama.h"

static void read_yama(int fd) {
  char buf[80];
  int bytes_read;
  do {
    bytes_read = read(fd, buf, sizeof(buf) - 1);
    buf[bytes_read] = '\0';
    printf("%s", buf);
  } while (bytes_read == sizeof(buf) - 1);
}

static void write_yama(int fd, char *record) {
  write(fd, record, strlen(record));
}

int main(int argc, char *argv[]) {
  fprintf(stderr, "Opening %s\n", argv[2]);
  int fd = open(argv[2], O_RDWR | O_CREAT,
		S_IWUSR | S_IRUSR);
  if (fd == -1) {
    perror("open");
    exit(-1);
  }

  off_t end = lseek(fd, 0, SEEK_END);
  lseek(fd, 0, SEEK_SET);
  YAMA *yama, _header;
  if (end == 0) {
    yama = yama_new();
    write(fd, yama, sizeof(YAMA));
  } else {
    yama = &_header;
    read(fd, yama, sizeof(YAMA));
  }

  read_yama(fd);

  if (strcmp(argv[1], "write") == 0) {
    write_yama(fd, argv[3]);
  }
  close(fd);
  return 0;
}
