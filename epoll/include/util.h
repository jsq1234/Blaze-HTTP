#ifndef util_h
#define util_h

#include <sys/types.h>
#include <stdint.h>
#include <errno.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <sys/epoll.h>

#define BUF_SIZE 512

extern char buffer[BUF_SIZE];

int init_server(uint16_t port);
int set_socket_nonblocking(int sockfd);
int concat(char* dst, char* src, size_t max_len);
int init_epoll();
void add_event(int epoll_fd, int sockfd, struct epoll_event *event,uint32_t flags);
int send_all(int sockfd, char *msg);

#endif