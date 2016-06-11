#include <stdio.h>
#include <string.h>
#include "minunit.h"
#include "yama.h"

int tests_run;

static char *test_yama_header() {
  char *result = NULL;
  yama_header *header = yama_new();
  mu_assert("Yama header magic",
	    memcmp(header->magic, "YAMA", 4) == 0);
  return result;
}

static char *run_all_tests() {
  mu_run_test(test_yama_header);
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
