#include "framing.h"
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <errno.h>

static int write_all(int fd, const void* buf, size_t n) {
  const uint8_t* p = (const uint8_t*)buf;
  while (n) {
    ssize_t w = write(fd, p, n);
    if (w < 0) { if (errno == EINTR) continue; return -1; }
    p += (size_t)w; n -= (size_t)w;
  }
  return 0;
}

static int read_all(int fd, void* buf, size_t n) {
  uint8_t* p = (uint8_t*)buf;
  while (n) {
    ssize_t r = read(fd, p, n);
    if (r == 0) return -2; // EOF
    if (r < 0) { if (errno == EINTR) continue; return -1; }
    p += (size_t)r; n -= (size_t)r;
  }
  return 0;
}

int send_frame(int fd, const void* payload, uint32_t len) {
  uint32_t n = htonl(len);
  if (write_all(fd, &n, sizeof(n)) < 0) return -1;
  if (len == 0) return 0;
  return write_all(fd, payload, len);
}

int recv_frame_alloc(int fd, uint8_t** out_buf, uint32_t* out_len) {
  uint32_t n_be;
  int rc = read_all(fd, &n_be, sizeof(n_be));
  if (rc) return rc;
  uint32_t n = ntohl(n_be);
  uint8_t* buf = (uint8_t*)malloc(n ? n : 1);
  if (!buf) return -3;
  if (n) {
    rc = read_all(fd, buf, n);
    if (rc) { free(buf); return rc; }
  }
  *out_buf = buf; *out_len = n;
  return 0;
}
