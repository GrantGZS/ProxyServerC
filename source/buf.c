#include "buf.h"

// get data size in the buf
// initialize a cycle buffer
void buf_init(buf_t *b, size_t size) {
    b->rd = 0;
    b->wr = 0;
    b->size = size;
    b->data = malloc(size);
}

// free the buffer
void buf_destroy(buf_t *b) { free(b->data); }

// get the consecutive segment of data
uint8_t *buf_get_data(buf_t *b, size_t *size) {
  // check if all data were read
  if (b->rd == b->wr) {
      return NULL;
  }

  // calculate the max segment
  size_t rd_off = b->rd % b->size;
  if (size) {
    size_t wr_off = b->wr % b->size;
    if (wr_off > rd_off) {
        *size = wr_off - rd_off;
    } else {
        *size = b->size - rd_off;
    }
  }

  return b->data + rd_off;
}

// get the consecutive space in buffer
uint8_t *buf_get_space(buf_t *b, size_t *size) {
  if (b->wr - b->rd == b->size) {
    return NULL;
  }
  // calculate the max segment
  size_t wr_off = b->wr % b->size;
  if (size) {
    size_t rd_off = b->rd % b->size;
    if (rd_off > wr_off) {
        *size = rd_off - wr_off;
    } else {
        *size = b->size - wr_off;
    }
  }
  return b->data + wr_off;
}

// update the data size
void buf_update_data(buf_t *b, size_t change) { 
    b->wr += change; 
}

// update the space size
void buf_update_space(buf_t *b, size_t change) {
  b->rd += change;
  // all data are read, reset rd/wr to get the maximum free space
  if (b->rd == b->wr) {
    b->rd = 0;
    b->wr = 0;
  }
}
