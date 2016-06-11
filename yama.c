#include <stdlib.h>
#include <string.h>
#include "yama.h"

static char _magic[4] = {'Y', 'A', 'M', 'A'};

yama_header *yama_new() {
  yama_header *result;
  result = malloc(sizeof(yama_header));
  memcpy(result, _magic, sizeof(_magic));
  return result;
}

void *yama_first(const yama_header *header) {
  return NULL;
}

yama_record *yama_add(const yama_header *header,
		      const char *payload) {
  return NULL;
}
