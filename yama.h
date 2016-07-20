#ifndef _YAMA_H_
#define _YAMA_H_

#include <stdint.h>
#include <string.h>
#include <time.h>

typedef struct yama_s YAMA;
typedef struct yama_item_s yama_item;

#define ALIGN 4
#define PACKED __attribute__((packed, aligned(ALIGN)))

typedef size_t record_offt;

YAMA *yama_read(int fd);
void yama_release(YAMA *);

yama_item *yama_latest(YAMA *yama);
yama_item *yama_previous(yama_item *item);
yama_item *yama_before(yama_item *item, yama_item *history);
yama_item *yama_full_history(YAMA *yama);

yama_item *yama_add(YAMA *yama, char *data, size_t len);
yama_item *yama_insert_after(yama_item *prev, char *data, size_t len);
yama_item *yama_edit(yama_item *old, char *data, size_t len);
void yama_mark_done(yama_item *item);

#define yama_add_string(yama, string) ({	\
      char *_data = (string);			\
      yama_add(yama, _data, strlen(_data)); })

#define yama_insert_string_after(prev, string) ({	\
      char *_data = (string);				\
      yama_insert_after(prev, _data, strlen(_data)); })

#define yama_edit_string(old, string) ({	\
      char *_data = (string);			\
      yama_edit(old, _data, strlen(_data)); })

int size(yama_item *item);
const char *payload(yama_item *item);
int is_done(yama_item *item);
time_t timestamp(yama_item *item);

#endif  // _YAMA_H_
