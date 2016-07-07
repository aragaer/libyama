#ifndef _TEST_FILLS_H_
#define _TEST_FILLS_H_

#include <yama.h>

void sequential_fill(YAMA *yama, yama_record *items[], int count);
void striped_fill(YAMA *yama, yama_record *items[], int count);
void fill_update(YAMA *yama, yama_record *items[], int count);

#endif  // _TEST_FILLS_H_
