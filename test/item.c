#include <yama.h>
#include "basic.h"
#include "item.h"
#include "minunit.h"

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
  yama_release(yama);
  return NULL;
}

char *test_item() {
  mu_run_test(test_first_item);
  return NULL;
}
