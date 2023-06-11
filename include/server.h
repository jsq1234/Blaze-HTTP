#ifndef SERVER_HEADER
#define SERVER_HEADER


#include "../include/http.h"
#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdint.h>
#include <stdio.h>
#include <errno.h>
#include <error.h>

typedef struct server_ds {
    int sockfd;
    struct sockaddr_in info;
    char* reply; // reply buffer, 2048 bytes for now
} server_t;

extern server_t server;
int init_server(uint16_t port);
int make_socket_nonblocking(int fd);

size_t OK_reply(FILE* fptr,const char* file_path,http_t* request);
int send_all(int sockfd, size_t len, const char* reply);
#endif
