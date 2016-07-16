#include <yama.h>
#include "basic.h"
#include "item.h"
#include "minunit.h"

static char *test_first_item() {
  YAMA *yama = yama_new();
  yama_item *first = yama_first_item(yama);
  mu_assert("Should be null", first == NULL);
  yama_release(yama);
  return NULL;
}

char *test_item() {
  mu_run_test(test_first_item);
  return NULL;
}
