#include <string.h>
#include <yama.h>
#include "minunit.h"
#include "basic.h"

char *test_add_item() {
  YAMA *yama = yama_new();

  mu_assert("Is empty initially",
	    yama_first(yama) == NULL);

  yama_record *item = yama_add(yama, "Hello, world");
  mu_assert("Record created", item != NULL);
  mu_assert("Payload size is correct",
	    size(item) == strlen("Hello, world"));
  mu_assert("Payload contents are correct",
	    memcmp(payload(item), "Hello, world", size(item)) == 0);
  mu_assert("This is also a last one",
	    yama_next(yama, item) == NULL);
  mu_assert("Added is first",
	    yama_first(yama) == item);
  yama_record *item2 = yama_add(yama, "Item 2");
  mu_assert("Other record created", item2 != NULL);
  mu_assert("It points to previous",
	    yama_next(yama, item2) == item);
  yama_release(yama);
  return NULL;
}

char *test_simple_usage() {
  YAMA *yama = yama_new();
  yama_add(yama, "Hello, world");
  yama_add(yama, "Another record");

  yama_record *item;
  item = yama_first(yama);
  mu_assert("Another record",
	    strncmp(payload(item), "Another record", size(item)) == 0);
  item = yama_next(yama, item);
  mu_assert("Hello, world",
	    strncmp(payload(item), "Hello, world", size(item)) == 0);
  yama_release(yama);
  return NULL;
}

char *test_insert() {
  YAMA *yama = yama_new();
  yama_record *item = yama_add(yama, "Hello, world");
  yama_insert_after(yama, item, "Another record");

  item = yama_first(yama);
  mu_assert("Hello, world",
	    strncmp(payload(item), "Hello, world", size(item)) == 0);
  item = yama_next(yama, item);
  mu_assert("Inserted", item != NULL);
  mu_assert("Another record",
	    strncmp(payload(item), "Another record", size(item)) == 0);
  yama_release(yama);
  return NULL;
}

char *test_edit() {
  YAMA *yama = yama_new();
  yama_record *item = yama_add(yama, "Hello, world");
  yama_record *item2 = yama_edit(yama, item, "Howdy, world");
  mu_assert("Stored new text", item2 != NULL);
  mu_assert("It is not old one", item2 != item);

  item = yama_first(yama);
  mu_assert("Howdy, world",
	    strncmp(payload(item), "Howdy, world", size(item)) == 0);
  mu_assert("Last one", yama_next(yama, item) == NULL);
  yama_release(yama);
  return NULL;
}

char *test_add_binary_data() {
  YAMA *yama = yama_new();
  char buf[] = {1, 2, 3, 4, 5};
  yama_record *item = yama_add_binary(yama, buf, sizeof(buf));
  mu_assert("Item is actually created", item != NULL);
  mu_assert("Item is actually added", yama_first(yama) != NULL);
  mu_assert("Item has correct size", size(item) == sizeof(buf));
  mu_assert("Item contains the data", memcmp(payload(item), buf, size(item)) == 0);
  yama_release(yama);
  return NULL;
}

char *basic_tests() {
  mu_run_test(test_add_binary_data);
  return NULL;
}

