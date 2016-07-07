#include "fills.h"

void sequential_fill(YAMA *yama, yama_record *items[], int count) {
  int i;
  for (i = 0; i < count; i++)
    items[i] = yama_add_string(yama, "x");
}

void striped_fill(YAMA *yama, yama_record *items[], int count) {
  int i;
  for (i = 1; i < count; i+=2)
    items[i] = yama_add_string(yama, "x");
  for (i = 0; i < count; i+=2)
    items[i] = yama_insert_string_after(yama, items[i+1], "x");
}

void fill_update(YAMA *yama, yama_record *items[], int count) {
  sequential_fill(yama, items, count);
  items[1] = yama_edit_string(yama, items[1], "y");
}
