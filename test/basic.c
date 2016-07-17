#include <string.h>
#include <yama.h>
#include "minunit.h"
#include "basic.h"
#include "util.h"

char *test_add_item() {
  YAMA *yama = yama_new();

  mu_assert("Is empty initially",
	    yama_first_item(yama) == NULL);

  yama_record *record = yama_add_string(yama, "Hello, world");
  yama_item *item = yama_first_item(yama);
  mu_assert("Record created", item != NULL);
  mu_assert("Payload size is correct",
	    item_size(item) == strlen("Hello, world"));
  mu_assert("Payload contents are correct",
	    memcmp(item_payload(item), "Hello, world",
		   item_size(item)) == 0);
  mu_assert("Added is first", get_record(item) == record);
  mu_assert("This is also a last one",
	    yama_next_item(item) == NULL);
  yama_add_string(yama, "Item 2");
  yama_item *item2 = yama_first_item(yama);
  mu_assert("Other record created", item2 != item);
  mu_assert("It points to previous", yama_next_item(item2) == item);
  yama_release(yama);
  return NULL;
}

char *test_simple_usage() {
  YAMA *yama = yama_new();
  yama_add_string(yama, "Hello, world");
  yama_add_string(yama, "Another record");

  yama_item *item = yama_first_item(yama);
  mu_assert("Another record",
	    strncmp(item_payload(item), "Another record",
		    item_size(item)) == 0);
  item = yama_next_item(item);
  mu_assert("Hello, world",
	    strncmp(item_payload(item), "Hello, world",
		    item_size(item)) == 0);
  yama_release(yama);
  return NULL;
}

char *test_insert() {
  YAMA *yama = yama_new();
  yama_record *record = yama_add_string(yama, "Hello, world");
  yama_insert_string_after(yama, record, "Another record");

  yama_item *item = yama_first_item(yama);
  mu_assert("Hello, world",
	    strncmp(item_payload(item), "Hello, world",
		    item_size(item)) == 0);
  item = yama_next_item(item);
  mu_assert("Inserted", item != NULL);
  mu_assert("Another record",
	    strncmp(item_payload(item), "Another record",
		    item_size(item)) == 0);
  yama_release(yama);
  return NULL;
}

char *test_edit() {
  YAMA *yama = yama_new();
  yama_record *record = yama_add_string(yama, "Hello, world");
  yama_record *record2 = yama_edit_string(yama, record, "Howdy, world");
  mu_assert("Stored new text", record2 != NULL);
  mu_assert("It is not old one", record2 != record);

  yama_item *item = yama_first_item(yama);
  mu_assert("Howdy, world",
	    strncmp(item_payload(item), "Howdy, world",
		    item_size(item)) == 0);
  mu_assert("Last one", yama_next_item(item) == NULL);
  yama_release(yama);
  return NULL;
}

char *test_binary_add() {
  YAMA *yama = yama_new();
  char buf[] = {1, 2, 3, 4, 5};
  yama_item *item = yama_add(yama, buf, sizeof(buf));
  mu_assert("Item is actually created", item != NULL);
  mu_assert("Item is actually added", yama_first_item(yama) == item);
  mu_assert("Item has correct size", item_size(item) == sizeof(buf));
  mu_assert("Item contains the data", memcmp(item_payload(item),
					     buf, item_size(item)) == 0);
  yama_release(yama);
  return NULL;
}

char *test_binary_insert() {
  YAMA *yama = yama_new();
  char buf[] = {1, 2, 3, 4, 5};
  yama_record *record1 = get_record(yama_add(yama, buf, sizeof(buf)));
  yama_record *record2 = yama_insert_after(yama, record1,
					   "Hello, world", strlen("Hello"));
  yama_item *item = yama_first_item(yama);
  mu_assert("Item is actually created", record2 != NULL);
  mu_assert("Item is not first", get_record(item) == record1);
  mu_assert("Item has correct size", size(record2) == strlen("Hello"));
  mu_assert("Item contains the data", memcmp(payload(record2), "Hello",
					     strlen("Hello")) == 0);
  yama_release(yama);
  return NULL;
}

char *test_binary_edit() {
  YAMA *yama = yama_new();
  char buf[] = {1, 2, 3, 4, 5};
  yama_record *record1 = get_record(yama_add(yama, buf, sizeof(buf)));
  yama_record *record2 = yama_edit(yama, record1,
				   "Hello, world", strlen("Hello"));
  yama_item *item = yama_first_item(yama);
  mu_assert("Item is actually created", record2 != NULL);
  mu_assert("Item is actually added", get_record(item) == record2);
  mu_assert("Item has correct size", size(record2) == strlen("Hello"));
  mu_assert("Item contains the data", memcmp(payload(record2), "Hello",
					     strlen("Hello")) == 0);
  mu_assert("This is also last one", yama_next_item(item) == NULL);
  mu_assert("Old one is still here", yama_before(record2, record2) == record1);
  yama_release(yama);
  return NULL;
}

char *test_mark_done() {
  YAMA *yama = yama_new();
  char buf[] = {1, 2, 3, 4, 5};
  yama_item *item = yama_add(yama, buf, sizeof(buf));
  yama_mark_done(yama, get_record(item));
  mu_assert("No items", yama_first_item(yama) == NULL);
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
