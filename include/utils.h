#ifndef UTILS_HEADER
#define UTILS_HEADER

#include <stdint.h>

// initilize a server with port
int init_server(uint16_t port);
int make_socket_nonblocking(int fd);

#endif
