#pragma once
#include <stdint.h>

int mk_listen(uint16_t port);            // returns server fd
int mk_accept(int server_fd);            // returns client fd
int mk_connect(const char* host, uint16_t port); // returns client fd
