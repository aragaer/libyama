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
int verbose;

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
	    size(item) == strlen("Hello, world"));
  mu_assert("Payload contents are correct",
	    memcmp(payload(item), "Hello, world",
		   size(item)) == 0);
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
	    strncmp(payload(item), "Another record",
		    size(item)) == 0);
  item = yama_next(yama, item);
  mu_assert("Hello, world",
	    strncmp(payload(item), "Hello, world",
		    size(item)) == 0);
  yama_release(yama);
  return NULL;
}

static char *test_file_create() {
  char template[] = "yamaXXXXXX";
  int fd = mkstemp(template);
  YAMA *yama = yama_read(fd);
  mu_assert("Open empty file", yama != NULL);
  char magic[4];
  lseek(fd, 0, SEEK_SET);
  int read_result = read(fd, magic, 4);
  mu_assert("Should be able to read first 4 bytes",
	    read_result == 4);
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
	    strncmp(payload(item), "Hello, world",
		    size(item)) == 0);
  item = yama_next(yama2, item);
  mu_assert("Not empty", item != NULL);
  mu_assert("Initial item",
	    strncmp(payload(item), "Initial item",
		    size(item)) == 0);
  struct stat buf;
  fstat(fd, &buf);
  mu_assert("File size is correct",
	    buf.st_size > strlen("Initial item")+strlen("Hello, world"));
  yama_release(yama);
  close(fd);
  unlink(template);
  return NULL;
}

static char *test_insert() {
  YAMA *yama = yama_new();
  yama_record *item = yama_add(yama, "Hello, world");
  yama_insert_after(yama, item, "Another record");

  item = yama_first(yama);
  mu_assert("Hello, world",
	    strncmp(payload(item), "Hello, world",
		    size(item)) == 0);
  item = yama_next(yama, item);
  mu_assert("Inserted",
	    item != NULL);
  mu_assert("Another record",
	    strncmp(payload(item), "Another record",
		    size(item)) == 0);
  yama_release(yama);
  return NULL;
}

static char *test_edit() {
  YAMA *yama = yama_new();
  yama_record *item = yama_add(yama, "Hello, world");
  yama_record *item2 = yama_edit(yama, item, "Howdy, world");
  mu_assert("Stored new text", item2 != NULL);
  mu_assert("It is not old one", item2 != item);

  item = yama_first(yama);
  mu_assert("Howdy, world",
	    strncmp(payload(item), "Howdy, world",
		    size(item)) == 0);
  mu_assert("Last one",
	    yama_next(yama, item) == NULL);
  yama_release(yama);
  return NULL;
}

static char *verify_integrity(YAMA *yama, yama_record *items[], int count) {
  mu_assert("Last one is in front",
	    yama_first(yama) == items[count-1]);
  int i;
  for (i = 0; i < count; i++) {
    yama_record *expected_next = i == 0 ? NULL : items[i-1];
    yama_record *expected_prev = i == count-1 ? NULL : items[i+1];
    mu_assert("Check next",
	      yama_next(yama, items[i]) == expected_next);
    mu_assert("Check previous",
	      yama_prev(yama, items[i]) == expected_prev);
  }
  return NULL;
}

static void sequential_fill(YAMA *yama, yama_record *items[], int count) {
  int i;
  for (i = 0; i < count; i++)
    items[i] = yama_add(yama, "x");
}

static void striped_fill(YAMA *yama, yama_record *items[], int count) {
  int i;
  for (i = 1; i < count; i+=2)
    items[i] = yama_add(yama, "x");
  for (i = 0; i < count; i+=2)
    items[i] = yama_insert_after(yama, items[i+1], "x");
}

static void fill_update(YAMA *yama, yama_record *items[], int count) {
  sequential_fill(yama, items, count);
  items[1] = yama_edit(yama, items[1], "y");
}

#define RECORDS 4
static char *test_traverse(void (*fill_func)(YAMA *, yama_record *[], int)) {
  YAMA *yama = yama_new();
  yama_record *items[RECORDS];
  fill_func(yama, items, RECORDS);

  char *verification_result = verify_integrity(yama, items, RECORDS);
  yama_release(yama);
  return verification_result;
}

static char *test_history() {
  YAMA *yama = yama_new();
  yama_record *items[RECORDS];
  fill_update(yama, items, RECORDS);

  mu_assert("First doesn't have previous version",
	    yama_before(items[0], items[0]) == NULL);

  yama_record *updated = items[1];
  yama_record *old = yama_before(updated, updated);
  mu_assert("Does have previous version", old != NULL);
  mu_assert("x", strncmp(payload(old), "x", size(old)) == 0);
  mu_assert("No older entry",
	    yama_before(updated, old) == NULL);

  yama_release(yama);
  return NULL;
}

static char *run_all_tests() {
  mu_run_test(test_yama_object);
  mu_run_test(test_add_item);
  mu_run_test(test_simple_usage);
  mu_run_test(test_file_create);
  mu_run_test(test_file_read_write);
  mu_run_test(test_insert);
  mu_run_test(test_edit);
  mu_run_test(test_traverse, sequential_fill);
  mu_run_test(test_traverse, striped_fill);
  mu_run_test(test_traverse, fill_update);
  mu_run_test(test_history);
  return NULL;
}

int main(int argc, char *argv[]) {
  if (argc > 1 && strncmp(argv[1], "-v", 2) == 0)
    sscanf(argv[1]+2, "%d", &verbose);
  char *result = run_all_tests();
  if (result == NULL)
    printf("All %d tests passed\n", tests_run);
  else
    printf("FAIL: %s\n", result);
  return result != NULL;
}
