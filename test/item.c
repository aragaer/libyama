#include <yama.h>
#include "item.h"
#include "minunit.h"
#include "util.h"

static char *test_first_item() {
  char *string = "Hello, world";
  YAMA *yama = yama_new();
  mu_assert("Should be empty", yama_first_item(yama) == NULL);

  yama_add_string(yama, string);
  yama_item *item = yama_first_item(yama);
  mu_assert("Should be not null", item != NULL);
  mu_assert("Should be correct size", item_size(item) == strlen(string));
  mu_assert("Should be correct payload",
	    memcmp(item_payload(item), string, strlen(string)) == 0);
  mu_assert("No next", yama_next_item(item) == NULL);
  mu_assert("First item is the same one",
	    yama_first_item(yama) == item);
  yama_release(yama);
  return NULL;
}

static char *test_item_traverse() {
  char *string = "Hello, world";
  YAMA *yama = yama_new();
  yama_add_string(yama, string);
  yama_add_string(yama, string);
  yama_item *item = yama_first_item(yama);
  yama_relocate(yama);
  yama_item *item2 = yama_next_item(item);
  mu_assert("Second item is not null", item2 != NULL);
  yama_release(yama);
  return NULL;
}

char *test_item() {
  mu_run_test(test_first_item);
  mu_run_test(test_item_traverse);
  return NULL;
}
