#ifndef UTILS_HEADER
#define UTILS_HEADER

#include <stdint.h>
#include <stdio.h>
// initilize a server with port
int init_server(uint16_t port);
int make_socket_nonblocking(int fd);
int read_text_file(FILE* fptr, char* buf);
#endif
