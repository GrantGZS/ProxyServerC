#include "sockutils.h"
#include "utils.h"
#include <assert.h>
#include <fcntl.h>
#include <poll.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include "buf.h"
#include "tcpproxy.h"

// local declarations
static int proxy_serve(proxy_t *p);
static int make_async(int s);
static int proxy_read_client(proxy_t *p);
static int proxy_write_client(proxy_t *p);
static int proxy_read_host(proxy_t *p);
static int proxy_write_host(proxy_t *p);

// start the proxy service
int start_proxy(char *remote_host, char *remote_port, char *proxy_port) {
  int ret;
  proxy_t proxy;

  // open socket for listening 
  int listenfd = make_server(proxy_port, 10);
  if (listenfd < 0) {
    die("Failed to open port %s for listening\n", proxy_port);
  }

  // prepare the host infomration
  struct addrinfo *addrinfo = make_addrinfo(remote_host, remote_port);
  while (true) {
    // accept the a client session
    int fd = server_accept(listenfd);
    if (fd < 0 || make_async(fd) < 0) {
      die("Failed to accept a client\n");
    }
    proxy.cfd = fd;

    // initialize buffers for the session
    buf_init(&proxy.cbuf, BUF_SZ);
    buf_init(&proxy.hbuf, BUF_SZ);

    // session loop until the client disconnects
    while (true) {
      // connect to host
      fd = host_connect(addrinfo);
      if (fd < 0 || make_async(fd) < 0) {
        die("Failed to connect to the remote host\n");
      }
      proxy.hfd = fd;

      // serve for one time
      ret = proxy_serve(&proxy);
      close(proxy.hfd);
      if (ret <= 0) {
        break;
      }
    }

    // clean up a client session
    close(proxy.cfd);
    buf_destroy(&proxy.cbuf);
    buf_destroy(&proxy.hbuf);
  }

  return ret;
}


// serve one time
static int proxy_serve(proxy_t *p) {
  struct pollfd pollfds[2];
  pollfds[0].fd = p->cfd;
  pollfds[1].fd = p->hfd;

  // initialze control variables
  int ret = 0;
  bool client_over = false;
  bool host_over = false;

  // async loop
  while (true) {
    pollfds[0].events = pollfds[0].revents = 0;
    pollfds[1].events = pollfds[1].revents = 0;

    // check if data from client possible and space for it available
    if (!client_over && buf_get_space(&p->cbuf, NULL)) {
      pollfds[0].events |= POLLIN;
    }
    // check if data to client available
    if (buf_get_data(&p->hbuf, NULL)) {
      pollfds[0].events |= POLLOUT;
    }
    // check if data from host possible and space for it available
    if (!host_over && buf_get_space(&p->hbuf, NULL)) {
      pollfds[1].events |= POLLIN;
    }
    // check if data to host available
    if (buf_get_data(&p->cbuf, NULL)) {
      pollfds[1].events |= POLLOUT;
    }

    // poll
    if (poll(pollfds, 2, -1) < 0) {
      pdie("poll");
    }

    // handle the event
    if (pollfds[0].revents & POLLIN) {
      ret = proxy_read_client(p);
      LOG("read_client = %d\n", ret);
      if (ret == 0) {
        client_over = true;
      }
    }
    if (pollfds[0].revents & POLLOUT) {
      ret = proxy_write_client(p);
      LOG("write_client = %d\n", ret);
    }
    if (pollfds[1].revents & POLLIN) {
      ret = proxy_read_host(p);
      LOG("read_host = %d\n", ret);
      if (ret == 0) {
        host_over = true;
      }
    }
    if (pollfds[1].revents & POLLOUT) {
      ret = proxy_write_host(p);
      LOG("write_host = %d\n", ret);
    }

    // if client disconnets, get out
    if (ret < 0 || client_over) {
      break;
    }

    // if host disconnets and all its response has been sent
    // back to client, get out
    if (host_over && !buf_get_data(&p->hbuf, NULL)) {
      ret = 1;
      break;
    }
  }

  return ret;
}


// read data from client
static int proxy_read_client(proxy_t *p) {
  size_t size;
  uint8_t *space = buf_get_space(&p->cbuf, &size);
  assert(space);

  ssize_t n = read(p->cfd, space, size);
  if (n > 0) {
    buf_update_data(&p->cbuf, n);
  }
  return n;
}

// write data to client
static int proxy_write_client(proxy_t *p) {
  size_t size;
  uint8_t *data = buf_get_data(&p->hbuf, &size);
  assert(data);

  ssize_t n = write(p->cfd, data, size);
  if (n > 0) {
    buf_update_space(&p->hbuf, n);
  }
  return n;
}

// read data from host
static int proxy_read_host(proxy_t *p) {
  size_t size;
  uint8_t *space = buf_get_space(&p->hbuf, &size);
  assert(space);

  ssize_t n = read(p->hfd, space, size);
  if (n > 0) {
    buf_update_data(&p->hbuf, n);
  }
  return n;
}

// write data to host
static int proxy_write_host(proxy_t *p) {
  size_t size;
  uint8_t *data = buf_get_data(&p->cbuf, &size);
  assert(data);

  ssize_t n = write(p->hfd, data, size);
  if (n > 0) {
    buf_update_space(&p->cbuf, n);
  }

  return n;
}

// make a socket as async
static int make_async(int s) {
  int n;
  if ((n = fcntl(s, F_GETFL)) == -1 ||
      fcntl(s, F_SETFL, n | O_NONBLOCK) == -1) {
    perr("fcntl");
  }
  n = 1;
  if (setsockopt(s, SOL_SOCKET, SO_KEEPALIVE, &n, sizeof(n)) == -1) {
    perr("setsockopt");
  }
  return 0;
err:
  return -1;
}