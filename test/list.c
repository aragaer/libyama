#include <list.h>
#include "minunit.h"

#define n_elem(a) sizeof(a)/sizeof(*a)

struct test_struct {
  list_head list;
};

static char *test_create_empty_list() {
  list_head head = LIST_HEAD_INIT();
  mu_assert("there is no first item",
	    list_get_next(&head) == &head);
  return NULL;
}

static char *test_insert_one_item() {
  list_head head = LIST_HEAD_INIT();
  struct test_struct s;
  list_add(&s.list, &head);
  mu_assert("first item is added one",
	    list_get_next(&head) == &s.list);
  mu_assert("there is no second item",
	    list_get_next(&s.list) == &head);
  return NULL;
}

static char *test_insert_item_in_front() {
  list_head head = LIST_HEAD_INIT();
  struct test_struct s1, s2;
  list_add(&s1.list, &head);
  list_add(&s2.list, &head);
  mu_assert("first item is new one",
	    list_get_next(&head) == &s2.list);
  mu_assert("second item is old one",
	    list_get_next(&s2.list) == &s1.list);
  mu_assert("there is no third item",
	    list_get_next(&s1.list) == &head);
  return NULL;
}

static char *test_insert_item_after_another() {
  list_head head = LIST_HEAD_INIT();
  struct test_struct s1, s2;
  list_add(&s1.list, &head);
  list_insert(&s2.list, &s1.list);
  mu_assert("first item is old one",
	    list_get_next(&head) == &s1.list);
  mu_assert("second item is new one",
	    list_get_next(&s1.list) == &s2.list);
  mu_assert("there is no third item",
	    list_get_next(&s2.list) == &head);
  return NULL;
}

static char *test_insert_item_in_the_end() {
  list_head head = LIST_HEAD_INIT();
  struct test_struct s1, s2;
  list_add(&s1.list, &head);
  list_add_tail(&s2.list, &head);
  mu_assert("first item is the same as before",
	    list_get_next(&head) == &s1.list);
  mu_assert("last item is the new one",
	    list_get_next(&s1.list) == &s2.list);
  return NULL;
}

static char *test_replace_item() {
  list_head head = LIST_HEAD_INIT();
  struct test_struct s1, s2;
  list_add(&s1.list, &head);
  list_replace(&s2.list, &s1.list);
  mu_assert("first item is new one",
	    list_get_next(&head) == &s2.list);
  mu_assert("there is no second item",
	    list_get_next(&s2.list) == &head);
  return NULL;
}

static char *test_remove_item() {
  list_head head = LIST_HEAD_INIT();
  struct test_struct s1;
  list_add(&s1.list, &head);
  list_remove(&s1.list);
  mu_assert("there is no first item",
	    list_get_next(&head) == &head);
  return NULL;
}

static char *test_replace2() {
  list_head head = LIST_HEAD_INIT();
  struct test_struct s[3], s2;
  int i;
  for (i = 0; i < n_elem(s); i++)
    list_add(&s[i].list, &head);
  mu_assert("first item is the one added last",
	    list_get_next(&head) == &s[2].list);
  mu_assert("second item is the one added before last",
	    list_get_next(&s[2].list) == &s[1].list);

  list_replace(&s2.list, &s[1].list);

  mu_assert("first item is the one added last",
	    list_get_next(&head) == &s[2].list);
  mu_assert("second item is the one updated",
	    list_get_next(&s[2].list) == &s2.list);
  return NULL;
}

static char *test_replace3() {
  list_head head = LIST_HEAD_INIT();
  struct test_struct s[3];
  int i;
  list_add(&s[0].list, &head);
  for (i = 1; i < n_elem(s); i++)
    list_replace(&s[i].list, &s[i-1].list);
  mu_assert("first item is the one added last",
	    list_get_next(&head) == &s[2].list);
  mu_assert("there is no second item",
	    list_get_next(&s[2].list) == &head);
  return NULL;
}

char *list_tests() {
  mu_run_test(test_create_empty_list);
  mu_run_test(test_insert_one_item);
  mu_run_test(test_insert_item_in_front);
  mu_run_test(test_insert_item_after_another);
  mu_run_test(test_insert_item_in_the_end);
  mu_run_test(test_replace_item);
  mu_run_test(test_remove_item);
  mu_run_test(test_replace2);
  mu_run_test(test_replace3);
  return NULL;
}
