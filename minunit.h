/* file: minunit.h */
#define mu_assert(message, test) do {		\
    printf("checking " #test "\n");		\
    if (!(test))				\
      return message;				\
  } while (0)

#define mu_run_test(test) do {			\
    printf("running " #test "\n");		\
    char *message = test();			\
    tests_run++;				\
    if (message)				\
      return message;				\
  } while (0)

extern int tests_run;
