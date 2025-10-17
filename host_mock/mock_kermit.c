#include "mock_kermit.h"
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>

int mk_listen(uint16_t port) {
  int fd = socket(AF_INET, SOCK_STREAM, 0);
  if (fd < 0) return -1;
  int one = 1;
  setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &one, sizeof(one));
  struct sockaddr_in addr; memset(&addr, 0, sizeof(addr));
  addr.sin_family = AF_INET;
  addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
  addr.sin_port = htons(port);
  if (bind(fd, (struct sockaddr*)&addr, sizeof(addr)) < 0) { close(fd); return -1; }
  if (listen(fd, 1) < 0) { close(fd); return -1; }
  return fd;
}

int mk_accept(int server_fd) {
  return accept(server_fd, NULL, NULL);
}

int mk_connect(const char* host, uint16_t port) {
  int fd = socket(AF_INET, SOCK_STREAM, 0);
  if (fd < 0) return -1;
  struct sockaddr_in addr; memset(&addr, 0, sizeof(addr));
  addr.sin_family = AF_INET;
  addr.sin_port = htons(port);
  if (!host) host = "127.0.0.1";
  if (inet_pton(AF_INET, host, &addr.sin_addr) != 1) { close(fd); return -1; }
  if (connect(fd, (struct sockaddr*)&addr, sizeof(addr)) < 0) { close(fd); return -1; }
  return fd;
}
