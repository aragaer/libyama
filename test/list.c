#include <list.h>
#include "minunit.h"

struct test_struct {
  int data;
  list_head list;
  list_head list2;
};

static char *test_create_empty_list() {
  list_head head = LIST_HEAD_INIT();
  mu_assert("is empty", list_is_empty(&head));
  mu_assert("there is no first item",
	    list_get_next(&head, &head) == NULL);
  return NULL;
}

static char *test_insert_one_item() {
  list_head head = LIST_HEAD_INIT();
  struct test_struct s = {1};
  list_add(&s.list, &head);
  mu_assert("not empty", !list_is_empty(&head));
  mu_assert("first item is added one",
	    list_get_next(&head, &head) == &s.list);
  mu_assert("there is no second item",
	    list_get_next(&s.list, &head) == NULL);
  return NULL;
}

static char *test_insert_item_in_front() {
  list_head head = LIST_HEAD_INIT();
  struct test_struct s1 = {1};
  struct test_struct s2 = {1};
  list_add(&s1.list, &head);
  list_add(&s2.list, &head);
  mu_assert("first item is new one",
	    list_get_next(&head, &head) == &s2.list);
  mu_assert("second item is old one",
	    list_get_next(&s2.list, &head) == &s1.list);
  mu_assert("there is no third item",
	    list_get_next(&s1.list, &head) == NULL);
  return NULL;
}

static char *test_insert_item_after_another() {
  list_head head = LIST_HEAD_INIT();
  struct test_struct s1 = {1};
  struct test_struct s2 = {1};
  list_add(&s1.list, &head);
  list_insert(&s2.list, &s1.list);
  mu_assert("first item is old one",
	    list_get_next(&head, &head) == &s1.list);
  mu_assert("second item is new one",
	    list_get_next(&s1.list, &head) == &s2.list);
  mu_assert("there is no third item",
	    list_get_next(&s2.list, &head) == NULL);
  return NULL;
}

static char *test_replace_item() {
  list_head head = LIST_HEAD_INIT();
  struct test_struct s1 = {1};
  struct test_struct s2 = {1};
  list_add(&s1.list, &head);
  list_replace(&s2.list, &s1.list);
  mu_assert("first item is new one",
	    list_get_next(&head, &head) == &s2.list);
  mu_assert("there is no second item",
	    list_get_next(&s2.list, &head) == NULL);
  return NULL;
}

static char *test_remove_item() {
  list_head head = LIST_HEAD_INIT();
  struct test_struct s1 = {1};
  list_add(&s1.list, &head);
  list_remove(&s1.list);
  mu_assert("is empty", list_is_empty(&head));
  mu_assert("there is no first item",
	    list_get_next(&head, &head) == NULL);
  return NULL;
}

static char *test_replace2() {
  list_head head = LIST_HEAD_INIT();
  struct test_struct s[3];
  struct test_struct s2;
  int i;
  for (i = 0; i < 3; i++)
    list_add(&s[i].list, &head);
  mu_assert("first item is the one added last",
	    list_get_next(&head, &head) == &s[2].list);
  mu_assert("second item is the one added before last",
	    list_get_next(&s[2].list, &head) == &s[1].list);

  list_replace(&s2.list, &s[1].list);

  mu_assert("first item is the one added last",
	    list_get_next(&head, &head) == &s[2].list);
  mu_assert("second item is the one updated",
	    list_get_next(&s[2].list, &head) == &s2.list);
  return NULL;
}

char *list_tests() {
  mu_run_test(test_create_empty_list);
  mu_run_test(test_insert_one_item);
  mu_run_test(test_insert_item_in_front);
  mu_run_test(test_insert_item_after_another);
  mu_run_test(test_replace_item);
  mu_run_test(test_remove_item);
  mu_run_test(test_replace2);
  return NULL;
}
