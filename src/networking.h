#ifndef NETWORK_HEADER
#define NETWORK_HEADER

#include <strings.h>
#include <sys/socket.h>
#include <stdint.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>

int bz_create_socket(const char* sock_type);
void bz_bind_socket(int fd, struct sockaddr_in* sadr);
void bz_start_listening(int fd, int backlog);

int bz_set_socket_nonblocking(int fd);
int bz_set_tcp_keepalive(int fd);
int bz_set_tcp_nodelay(int fd);
int bz_set_so_reuse_port(int fd);
int bz_set_so_reuse_addr(int fd);

int bz_accept(int fd, struct sockaddr* adr, socklen_t* len);

#endif