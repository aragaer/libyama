#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>
#include <yama.h>
#include "files.h"
#include "minunit.h"

char *test_file_create() {
  char template[] = "yamaXXXXXX";
  int fd = mkstemp(template);
  YAMA *yama = yama_read(fd);
  mu_assert("Open empty file", yama != NULL);
  char magic[4];
  lseek(fd, 0, SEEK_SET);
  int read_result = read(fd, magic, 4);
  mu_assert("Should be able to read first 4 bytes", read_result == 4);
  mu_assert("File should start with magic",
	    memcmp(magic, "YAMA", 4) == 0);
  unlink(template);
  yama_release(yama);
  close(fd);
  return NULL;
}

char *test_file_read_write() {
  char template[] = "yamaXXXXXX";
  int fd = mkstemp(template);
  YAMA *yama = yama_read(fd);
  yama_add_string(yama, "Initial item");
  yama_add_string(yama, "Hello, world");
  yama_release(yama);
  close(fd);

  fd = open(template, O_RDWR);
  YAMA *yama2 = yama_read(fd);
  yama_item *item = yama_latest(yama2);
  mu_assert("Not empty", item != NULL);
  mu_assert("Hello, world",
	    strncmp(payload(item), "Hello, world", size(item)) == 0);
  item = yama_previous(item);
  mu_assert("Not empty", item != NULL);
  mu_assert("Initial item",
	    strncmp(payload(item), "Initial item", size(item)) == 0);
  struct stat buf;
  fstat(fd, &buf);
  mu_assert("File size is correct",
	    buf.st_size > strlen("Initial item")+strlen("Hello, world"));
  yama_release(yama);
  close(fd);
  unlink(template);
  return NULL;
}
