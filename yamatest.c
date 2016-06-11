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
  mu_assert("Record created",
	    item != NULL);
  mu_assert("Payload size is correct",
	    item->size == strlen("Hello, world"));
  mu_assert("Payload contents are correct",
	    memcmp(item->payload, "Hello, world",
		   item->size) == 0);
  mu_assert("This is also a last one",
	    item->next == NULL);
  mu_assert("Added is first",
	    yama_first(header) == item);
  yama_record *item2 = yama_add(header, "Item 2");
  mu_assert("Other record created",
	    item2 != NULL);
  mu_assert("It points to previous",
	    item2->next == item);
  return NULL;
}

static char *test_simple_usage() {
  yama_header *header = yama_new();
  yama_add(header, "Hello, world");
  yama_add(header, "Another record");

  yama_record *item;
  item = yama_first(header);
  mu_assert("Another record",
	    strncmp(item->payload, "Another record",
		    item->size) == 0);
  item = item->next;
  mu_assert("Hello, world",
	    strncmp(item->payload, "Hello, world",
		    item->size) == 0);
  return NULL;
}

static char *run_all_tests() {
  mu_run_test(test_yama_header);
  mu_run_test(test_add_item);
  mu_run_test(test_simple_usage);
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
