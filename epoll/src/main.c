#include <arpa/inet.h>
#include <errno.h>
#include <fcntl.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#define MAX_EVENTS 64
#define BUF_SIZE 512
#define RES_BUF_SIZE 1024
#define REQ_BUF_SIZE 1024
#define DEBUG

#include "../include/util.h"

// int init_server(uint16_t port);
// int set_socket_nonblocking(int sockfd);
// int concat(char *restrict dst, char *restrict src, size_t n);

void accept_incoming_connection(int sockfd, int epoll_fd);
int read_all(int sockfd, char *msg, size_t buf_size);


int main(int argc, char *argv[]) {
  if (argc != 2) {
    fprintf(stderr, "Usage: %s [port]\n", argv[0]);
    exit(EXIT_FAILURE);
  }

  uint16_t port = atoi(argv[1]);
  int sockfd = init_server(port);
  int epoll_fd = init_epoll();

  struct epoll_event event;
  add_event(epoll_fd, sockfd, &event, EPOLLIN | EPOLLET);

  struct epoll_event events[MAX_EVENTS];
  memset(events, 0, sizeof(events));

  while (1) {
    printf("going back!\n");
    int nfds = epoll_wait(epoll_fd, events, MAX_EVENTS, -1);
    printf("got an event!\n");
    if (nfds == -1) {
      perror("epoll_wait()");
      exit(1);
    }
    for (int i = 0; i < nfds; i++) {
      if ((events[i].events & EPOLLERR) || (events[i].events & EPOLLHUP) ||
          (!(events[i].events & EPOLLIN))) {
        fprintf(stderr, "epoll error\n");
        close(events[i].data.fd);
        continue;
      }
      if (events[i].data.fd == sockfd) { // new connection!
        while (1) {
          struct sockaddr_in in_adr;
          socklen_t in_len = sizeof(in_adr);
          memset(&in_adr, 0, in_len);
          int in_fd = accept(sockfd, (struct sockaddr *)&in_adr, &in_len);
          if (in_fd == -1) {
            if (errno == EAGAIN || errno == EWOULDBLOCK)
              break;
            else {
              perror("accept()");
              break;
            }
          }
#ifdef DEBUG
          printf("new client!\n");
#endif
          add_event(epoll_fd, in_fd, &event, EPOLLIN | EPOLLET);
          continue;
        }
      } else {

        char msg[REQ_BUF_SIZE];
        memset(msg, 0, sizeof(msg));

        if (read_all(events[i].data.fd, msg, REQ_BUF_SIZE)) {
          printf("client closed connection!");
          close(events[i].data.fd);
          continue;
        }
        printf("writing to client");
        char res[RES_BUF_SIZE];
        memset(res, 0, sizeof(res));
        strcpy(res, "Hello World!");

        send_all(events[i].data.fd, res);
      }
    }
  }
}


int read_all(int sockfd, char *msg, size_t buf_size) {
  printf("reading from the client!\n");
  while (1) {
    ssize_t bytes = read(sockfd, buffer, BUF_SIZE - 1);
    printf("bytes read : %ld\n", bytes);
    if (bytes <= 0) {
      if (bytes == -1) {
        if (errno != EAGAIN) {
          perror("read()");
          return 1;
        }
        return 0;
      }
      else {
      //close(sockfd);
      return 1;
    }
    } 
    buffer[bytes] = '\0';
    printf("%s\n", buffer);
    if (concat(msg, buffer, buf_size) == -1) {
      fprintf(stderr, "buffer overflow\n");
      exit(1); // for now exit, will improve on this.
    }
    // if(strncmp(buffer + bytes - 4, "\r\n\r\n", 4) == 0) {
    //   break;
    // }
  }

  return 0;
}


