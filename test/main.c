#include <string.h>
#include "basic.h"
#include "files.h"
#include "list.h"
#include "minunit.h"
#include "traverse.h"
#include "yama.h"

int tests_run;
int verbose;

static void sequential_fill(YAMA *yama, yama_record *items[], int count) {
  int i;
  for (i = 0; i < count; i++)
    items[i] = yama_add_string(yama, "x");
}

static void striped_fill(YAMA *yama, yama_record *items[], int count) {
  int i;
  for (i = 1; i < count; i+=2)
    items[i] = yama_add_string(yama, "x");
  for (i = 0; i < count; i+=2)
    items[i] = yama_insert_string_after(yama, items[i+1], "x");
}

static void fill_update(YAMA *yama, yama_record *items[], int count) {
  sequential_fill(yama, items, count);
  items[1] = yama_edit_string(yama, items[1], "y");
}

#define RECORDS 4
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

static char *test_longer_history() {
  YAMA *yama = yama_new();
  int i;
  yama_record *item = NULL;
  for (i = 0; i < 5; i++)
    if (item == NULL)
      item = yama_add(yama, (char *) &i, sizeof(i));
    else
      item = yama_edit(yama, item, (char *) &i, sizeof(i));
  yama_record *first = yama_first(yama);
  for (i = 4, item = first; i >= 0; i--, item = yama_before(item, first)) {
    mu_assert("Size is correct", size(item) == sizeof(i));
    mu_assert("Contents are correct", memcmp(payload(item), &i, size(item)) == 0);
  }
  mu_assert("That was last in history", item == NULL);
  mu_assert("Just one record", yama_next(yama, first) == NULL);
  yama_release(yama);
  return NULL;
}

static char *run_all_tests() {
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
  mu_run_test(basic_tests);
  mu_run_test(list_tests);
  mu_run_test(test_longer_history);
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
