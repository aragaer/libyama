#include <stdio.h>
#include <string.h>
#include "minunit.h"
#include "yama.h"

int tests_run;

static char *test_yama_object() {
  YAMA *yama = yama_new();
  mu_assert("Yama yama magic",
	    memcmp(yama->magic, "YAMA", 4) == 0);
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
	    item->size == strlen("Hello, world"));
  mu_assert("Payload contents are correct",
	    memcmp(item->payload, "Hello, world",
		   item->size) == 0);
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
	    strncmp(item->payload, "Another record",
		    item->size) == 0);
  item = yama_next(yama, item);
  mu_assert("Hello, world",
	    strncmp(item->payload, "Hello, world",
		    item->size) == 0);
  yama_release(yama);
  return NULL;
}

static char *run_all_tests() {
  mu_run_test(test_yama_object);
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
