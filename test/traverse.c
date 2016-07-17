#include "minunit.h"
#include "traverse.h"
#include "util.h"

#define RECORDS 6

static char *verify_integrity(YAMA *yama, yama_item *items[], int count) {
  mu_assert("Last one is in front",
	    yama_first(yama) == items[count-1]);
  int i;
  for (i = 0; i < count; i++) {
    yama_item *expected_next = i == 0 ? NULL : items[i-1];
    mu_assert("Check next",
	      yama_next(items[i]) == expected_next);
    /*
    yama_record *expected_prev = i == count-1 ? NULL : records[i+1];
    mu_assert("Check previous",
	      yama_prev(yama, records[i]) == expected_prev);
    */
  }
  return NULL;
}

char *test_traverse(void (*fill_func)(YAMA *, yama_item *[], int)) {
  YAMA *yama = yama_new();
  yama_item *items[RECORDS];
  fill_func(yama, items, RECORDS);

  char *verification_result = verify_integrity(yama, items, RECORDS);
  yama_release(yama);
  return verification_result;
}
