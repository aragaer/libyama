#include <string.h>
#include <yama.h>
#include "minunit.h"
#include "basic.h"
#include "util.h"

char *test_add_item() {
  YAMA *yama = yama_new();

  mu_assert("Is empty initially",
	    yama_first(yama) == NULL);

  yama_item *item = yama_add_string(yama, "Hello, world");
  mu_assert("Record created", item != NULL);
  mu_assert("Payload size is correct",
	    size(item) == strlen("Hello, world"));
  mu_assert("Payload contents are correct",
	    memcmp(payload(item), "Hello, world",
		   size(item)) == 0);
  mu_assert("Added is first", item == yama_first(yama));
  mu_assert("This is also a last one",
	    yama_next(item) == NULL);
  yama_add_string(yama, "Item 2");
  yama_item *item2 = yama_first(yama);
  mu_assert("Other record created", item2 != item);
  mu_assert("It points to previous", yama_next(item2) == item);
  yama_release(yama);
  return NULL;
}

char *test_simple_usage() {
  YAMA *yama = yama_new();
  yama_add_string(yama, "Hello, world");
  yama_add_string(yama, "Another record");

  yama_item *item = yama_first(yama);
  mu_assert("Another record",
	    strncmp(payload(item), "Another record",
		    size(item)) == 0);
  item = yama_next(item);
  mu_assert("Hello, world",
	    strncmp(payload(item), "Hello, world",
		    size(item)) == 0);
  yama_release(yama);
  return NULL;
}

char *test_insert() {
  YAMA *yama = yama_new();
  yama_item *item = yama_add_string(yama, "Hello, world");
  yama_insert_string_after(item, "Another record");

  item = yama_first(yama);
  mu_assert("Hello, world",
	    strncmp(payload(item), "Hello, world",
		    size(item)) == 0);
  item = yama_next(item);
  mu_assert("Inserted", item != NULL);
  mu_assert("Another record",
	    strncmp(payload(item), "Another record",
		    size(item)) == 0);
  yama_release(yama);
  return NULL;
}

char *test_edit() {
  YAMA *yama = yama_new();
  yama_item *item = yama_add_string(yama, "Hello, world");
  yama_item *item2 = yama_edit_string(item, "Howdy, world");
  mu_assert("Stored new text", item2 != NULL);
  mu_assert("It is not old one", item2 != item);

  yama_item *first = yama_first(yama);
  mu_assert("Howdy, world",
	    strncmp(payload(first), "Howdy, world",
		    size(first)) == 0);
  mu_assert("Last one", yama_next(first) == NULL);
  yama_release(yama);
  return NULL;
}

char *test_binary_add() {
  YAMA *yama = yama_new();
  char buf[] = {1, 2, 3, 4, 5};
  yama_item *item = yama_add(yama, buf, sizeof(buf));
  mu_assert("Item is actually created", item != NULL);
  mu_assert("Item is actually added", yama_first(yama) == item);
  mu_assert("Item has correct size", size(item) == sizeof(buf));
  mu_assert("Item contains the data", memcmp(payload(item),
					     buf, size(item)) == 0);
  yama_release(yama);
  return NULL;
}

char *test_binary_insert() {
  YAMA *yama = yama_new();
  char buf[] = {1, 2, 3, 4, 5};
  yama_item *item1 = yama_add(yama, buf, sizeof(buf));
  yama_item *item2 = yama_insert_after(item1, "Hello, world", strlen("Hello"));
  mu_assert("Item is actually created", item2 != NULL);
  mu_assert("Item is not first", yama_first(yama) == item1);
  mu_assert("Item has correct size", size(item2) == strlen("Hello"));
  mu_assert("Item contains the data", memcmp(payload(item2), "Hello",
					     strlen("Hello")) == 0);
  yama_release(yama);
  return NULL;
}

char *test_binary_edit() {
  YAMA *yama = yama_new();
  char buf[] = {1, 2, 3, 4, 5};
  yama_item *item1 = yama_add(yama, buf, sizeof(buf));
  yama_item *item2 = yama_edit(item1, "Hello, world", strlen("Hello"));
  mu_assert("Item is actually created", item2 != NULL);
  mu_assert("Item is actually added", yama_first(yama) == item2);
  mu_assert("Item has correct size", size(item2) == strlen("Hello"));
  mu_assert("Item contains the data", memcmp(payload(item2), "Hello",
					     strlen("Hello")) == 0);
  mu_assert("This is also last one", yama_next(item2) == NULL);
  mu_assert("Old one is still here", yama_before(item2, item2) == item1);
  yama_release(yama);
  return NULL;
}

char *test_mark_done() {
  YAMA *yama = yama_new();
  char buf[] = {1, 2, 3, 4, 5};
  yama_item *item = yama_add(yama, buf, sizeof(buf));
  yama_mark_done(item);
  mu_assert("No items", yama_first(yama) == NULL);
  yama_release(yama);
  return NULL;
}

char *basic_tests() {
  mu_run_test(test_add_item);
  mu_run_test(test_simple_usage);
  mu_run_test(test_insert);
  mu_run_test(test_edit);
  mu_run_test(test_binary_add);
  mu_run_test(test_binary_insert);
  mu_run_test(test_binary_edit);
  mu_run_test(test_mark_done);
  return NULL;
}
