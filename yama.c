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
