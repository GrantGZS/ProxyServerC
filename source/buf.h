#ifndef __BUF_H__
#define __BUF_H__

#include <stdlib.h>
#include <stdint.h>

typedef struct buf {
  size_t rd; // next pos to read data
  size_t wr; // next pos to write data
  size_t size;
  uint8_t *data;
} buf_t;

extern void buf_init(buf_t *b, size_t size);
extern void buf_destroy(buf_t *b);
extern uint8_t *buf_get_data(buf_t *b, size_t *size);
extern uint8_t *buf_get_space(buf_t *b, size_t *size);
extern void buf_update_data(buf_t *b, size_t change);
extern void buf_update_space(buf_t *b, size_t change);

#endif
