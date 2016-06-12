#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include "minunit.h"
#include "yama.h"

int tests_run;

static char *test_yama_object() {
  YAMA *yama = yama_new();
  mu_assert("Is empty initially",
	    yama_first(yama) == NULL);
  yama_release(yama);
  return NULL;
}

static char *test_add_item() {
  YAMA *yama = yama_new();
  yama_record *item = yama_add(yama, "Hello, world");
  mu_assert("Record created",
	    item != NULL);
  mu_assert("Payload size is correct",
	    item->size == strlen("Hello, world"));
  mu_assert("Payload contents are correct",
	    memcmp(item->payload, "Hello, world",
		   item->size) == 0);
  mu_assert("This is also a last one",
	    yama_next(yama, item) == NULL);
  mu_assert("Added is first",
	    yama_first(yama) == item);
  yama_record *item2 = yama_add(yama, "Item 2");
  mu_assert("Other record created",
	    item2 != NULL);
  mu_assert("It points to previous",
	    yama_next(yama, item2) == item);
  yama_release(yama);
  return NULL;
}

static char *test_simple_usage() {
  YAMA *yama = yama_new();
  yama_add(yama, "Hello, world");
  yama_add(yama, "Another record");

  yama_record *item;
  item = yama_first(yama);
  mu_assert("Another record",
	    strncmp(item->payload, "Another record",
		    item->size) == 0);
  item = yama_next(yama, item);
  mu_assert("Hello, world",
	    strncmp(item->payload, "Hello, world",
		    item->size) == 0);
  yama_release(yama);
  return NULL;
}

static char *test_file_create() {
  char template[] = "yamaXXXXXX";
  int fd = mkstemp(template);
  YAMA *yama = yama_read(fd);
  mu_assert("Open empty file", yama != NULL);
  off_t end = lseek(fd, 0, SEEK_END);
  mu_assert("File should no longer be empty",
	    end >= sizeof(YAMA));
  char magic[4];
  lseek(fd, 0, SEEK_SET);
  read(fd, magic, 4);
  mu_assert("File should start with magic",
	    memcmp(magic, "YAMA", 4) == 0);
  unlink(template);
  yama_release(yama);
  close(fd);
  return NULL;
}

static char *test_file_read_write() {
  char template[] = "yamaXXXXXX";
  int fd = mkstemp(template);
  YAMA *yama = yama_read(fd);
  yama_add(yama, "Initial item");
  yama_add(yama, "Hello, world");
  yama_release(yama);
  close(fd);

  fd = open(template, O_RDWR);
  YAMA *yama2 = yama_read(fd);
  yama_record *item = yama_first(yama2);
  mu_assert("Not empty", item != NULL);
  mu_assert("Hello, world",
	    strncmp(item->payload, "Hello, world",
		    item->size) == 0);
  item = yama_next(yama2, item);
  mu_assert("Not empty", item != NULL);
  mu_assert("Initial item",
	    strncmp(item->payload, "Initial item",
		    item->size) == 0);
  struct stat buf;
  fstat(fd, &buf);
  mu_assert("File size is correct",
	    buf.st_size > strlen("Initial item")+strlen("Hello, world"));
  yama_release(yama);
  close(fd);
  unlink(template);
  return NULL;
}

static char *run_all_tests() {
  mu_run_test(test_yama_object);
  mu_run_test(test_add_item);
  mu_run_test(test_simple_usage);
  mu_run_test(test_file_create);
  mu_run_test(test_file_read_write);
  return NULL;
}

int main() {
  char *result = run_all_tests();
  if (result == NULL)
    printf("All %d tests passed\n", tests_run);
  else
    printf("FAIL: %s\n", result);
  return result != NULL;
}
