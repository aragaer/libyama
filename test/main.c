#include <string.h>
#include "basic.h"
#include "files.h"
#include "fills.h"
#include "item.h"
#include "list.h"
#include "minunit.h"
#include "traverse.h"
#include "util.h"
#include "yama.h"

int tests_run;
int verbose;

#define RECORDS 4
static char *test_history() {
  YAMA *yama = yama_new();
  yama_item *items[RECORDS];
  fill_update(yama, items, RECORDS);

  mu_assert("First doesn't have previous version",
	    yama_before(items[0], items[0]) == NULL);

  yama_item *updated = items[1];
  yama_item *old = yama_before(updated, updated);
  mu_assert("Does have previous version", old != NULL);
  mu_assert("x", strncmp(payload(old), "x", size(old)) == 0);
  mu_assert("No older entry", yama_before(updated, old) == NULL);

  yama_release(yama);
  return NULL;
}

static char *test_longer_history() {
  YAMA *yama = yama_new();
  int i = 0;
  yama_item *item = yama_add(yama, (char *) &i, sizeof(i));
  for (i = 1; i < RECORDS; i++)
    item = yama_edit(item, (char *) &i, sizeof(i));
  yama_item *first = yama_first(yama);
  for (item = first; item; item = yama_before(item, first)) {
    i--;
    mu_assert("Size is correct", size(item) == sizeof(i));
    mu_assert("Contents are correct", memcmp(payload(item), &i, size(item)) == 0);
  }
  mu_assert("That was last in history", i == 0);
  mu_assert("Just one record", yama_next(first) == NULL);
  yama_release(yama);
  return NULL;
}

static char *run_all_tests() {
  mu_run_test(test_file_create);
  mu_run_test(test_file_read_write);
  mu_run_test(test_traverse, sequential_fill);
  mu_run_test(test_traverse, striped_fill);
  mu_run_test(test_traverse, fill_update);
  mu_run_test(list_tests);
  mu_run_test(basic_tests);
  mu_run_test(test_history);
  mu_run_test(test_longer_history);
  mu_run_test(test_item);
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
