#include "../include/utils.h"
#include <arpa/inet.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/fcntl.h>
#include <sys/socket.h>
#include <unistd.h>

int init_server(uint16_t port) {

  int sockfd = socket(AF_INET, SOCK_STREAM, 0);
  if (sockfd < 0) {
    perror("socket()");
    exit(1);
  }

  struct sockaddr_in serv_addr;
  memset(&serv_addr, 0, sizeof(serv_addr));

  serv_addr.sin_family = AF_INET;
  serv_addr.sin_port = htons(port);
  serv_addr.sin_addr.s_addr = INADDR_ANY;

  if (bind(sockfd, (const struct sockaddr *)&serv_addr, sizeof(serv_addr)) <
      0) {
    perror("bind()");
    exit(1);
  }

  make_socket_nonblocking(sockfd);

  if (listen(sockfd, 5) < 0) {
    perror("listen()");
    exit(1);
  }
  return 0;
}

int make_socket_nonblocking(int fd) {
  int flags = fcntl(fd, F_GETFL, 0);
  if (flags < 0) {
    perror("fcntl()");
    exit(1);
  }
  flags |= O_NONBLOCK;

  if (fcntl(fd, F_SETFL, flags) < 0) {
    perror("fcntl()");
    exit(1);
  }
  return 0;
}
