#include <stdio.h>
#include <string.h>
#include "minunit.h"
#include "yama.h"

int tests_run;

static char *test_yama_header() {
  yama_header *header = yama_new();
  mu_assert("Yama header magic",
	    memcmp(header->magic, "YAMA", 4) == 0);

  mu_assert("Is empty initially",
	    yama_first(header) == NULL);
  return NULL;
}

static char *test_add_item() {
  yama_header *header = yama_new();
  yama_record *item = yama_add(header, "Hello, world");
  return NULL;
}

static char *run_all_tests() {
  mu_run_test(test_yama_header);
  mu_run_test(test_add_item);
  return NULL;
}

main() {
  char *result = run_all_tests();
  if (result == NULL)
    printf("All %d tests passed\n", tests_run);
  else
    printf("FAIL: %s\n", result);
  return result != NULL;
}
