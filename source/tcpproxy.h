#ifndef __TCP_PROXY_H__
#define __TCP_PROXY_H__

#include "buf.h"

#ifdef DEBUG
#define LOG(fmt, ...)                                                          \
  do {                                                                         \
    fprintf(stderr, "[LOG]" fmt, ##__VA_ARGS__);                               \
  } while (0)
#else
#define LOG(...)
#endif

#define BUF_SZ 256

typedef struct {
  int cfd;
  int hfd;
  buf_t cbuf;
  buf_t hbuf;
} proxy_t;

extern int start_proxy(char *remote_host, char *remote_port, char *proxy_port);

#endif // !__TCP_PROXY_H__