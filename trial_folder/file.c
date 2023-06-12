#include <arpa/inet.h>
#include <errno.h>
#include <fcntl.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <unistd.h>

#define MAX_EVENTS 100
#define RECV_SIZE 1024


// SERVER INITIALIZATION 

int init_server(uint16_t port, struct sockaddr_in *server);
int make_socket_nonblocking(int fd);

// HTTP REQUEST PARSING 

typedef struct http_headers {
  char connection[10];
  char content_type[30];
  char content_length[8];
} http_header_t;

typedef struct http_request {
  char method[10];
  char url[200];
  char version[10];
  http_header_t headers;
} http_t;


const char *not_found_reply = "HTTP/1.1 404 Not Found\r\n"
                              "Content-Length: 0\r\n\r\n";
const char *not_implemented_reply = "HTTP/1.1 203 Not implemented\r\n"
                                    "Content-Length: 0\r\n\r\n";

int parse_request(const char *msg, size_t len, http_t *request);

// SEND/RECV functions

int send_all(int sockfd, size_t len, const char *reply);


int main(int argc, char **argv) {

  if (argc < 2) {
    fprintf(stderr, "Usage : %s [port]\n", argv[0]);
  }

  struct sockaddr_in server;
  memset(&server, 0, sizeof(server));

  int listen_fd = init_server(atoi(argv[1]), &server);
  int epoll_fd = 0;
  struct epoll_event ev;
  struct epoll_event events[MAX_EVENTS];
  memset(events, 0, sizeof(events));
  memset(&ev, 0, sizeof(ev));

  ev.events = EPOLLIN;
  ev.data.fd = listen_fd;

  if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, listen_fd, &ev) < 0) {
    perror("epoll_ctl");
    exit(1);
  }

  int nfds = 0;
  struct sockaddr_in in_addr;
  socklen_t in_len = sizeof(in_addr);

  for (;;) {
    nfds = epoll_wait(epoll_fd, events, MAX_EVENTS, -1);

    if (nfds == -1) {
      perror("epoll_wait");
      exit(1);
    }

    for (int i = 0; i < nfds; i++) {
      if (events[i].events == EPOLLHUP) {
        fprintf(stderr, "got EPOLLUP\n");
      }
      if (events[i].events == EPOLLERR) {
        fprintf(stderr, "got EPOLLERR\n");
      }
      if (events[i].events == EPOLLIN) {
        if (events[i].data.fd == listen_fd) {
          // new connection has arrived, accept all of them
          while (1) {
            memset(&in_addr, 0, sizeof(in_addr));
            int conn_fd =
                accept(listen_fd, (struct sockaddr *)&in_addr, &in_len);
            if (conn_fd == -1) {
              if (errno == EAGAIN || errno == EWOULDBLOCK) {
                printf("accepted all connection.\n");
                break;
              }
              perror("accept()");
              continue;
            }

            if (make_socket_nonblocking(conn_fd) == -1) {
              continue;
            }

            ev.events = EPOLLIN | EPOLLET;
            ev.data.fd = conn_fd;

            if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, conn_fd, &ev) == -1) {
              fprintf(stderr, "epoll set insertion error: fd=%d", conn_fd);
              close(conn_fd);
              continue;
            }
          }
        } else {
          // a regular client
          int client_fd = events[i].data.fd;
          char buffer[RECV_SIZE];
          int client_closed = 0;

          while (1) {
            char *ptr = buffer;
            ssize_t bytes = recv(client_fd, ptr, RECV_SIZE - 1, 0);
            if (bytes == 0) {
              // client closed the connection
              printf("client %d closed the connection.\n", client_fd);
              client_closed = 1;
              close(client_fd);
              break;
            } else if (bytes == -1) {
              if (errno == EAGAIN || errno == EWOULDBLOCK) {
                // nothing more to read, we read everything.
                break;
              }
              perror("recv()");
              break;
            }else{
              ptr[bytes] = '\0';
              ptr += bytes;
            }
          }

          // if client hasn't closed the connection
          // only then send the reply 
          if( client_closed == 0 ){

          }
        }
      }
    }
  }
}

int make_socket_nonblocking(int fd) {
  int flags = fcntl(fd, F_GETFL, 0);
  if (flags < 0) {
    perror("fcntl()");
    return -1;
  }
  flags |= O_NONBLOCK;

  if (fcntl(fd, F_SETFL, flags) < 0) {
    perror("fcntl()");
    return -1;
  }
  return 0;
}

int init_server(uint16_t port, struct sockaddr_in *server) {
  int sockfd = socket(AF_INET, SOCK_STREAM, 0);
  if (sockfd < 0) {
    perror("socket()");
    exit(1);
  }
  memset(server, 0, sizeof(*server));

  server->sin_family = AF_INET;
  server->sin_port = htons(port);
  server->sin_addr.s_addr = INADDR_ANY;

  if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &(int){1}, sizeof(int)) <
      0) {
    perror("setsockopt()");
    exit(1);
  }

  if (bind(sockfd, (const struct sockaddr *)server, sizeof(*server)) < 0) {
    perror("bind()");
    exit(1);
  }

  if (make_socket_nonblocking(sockfd) == -1) {
    exit(1);
  }

  if (listen(sockfd, 100) < 0) {
    perror("listen()");
    exit(1);
  }
  printf("Server is listening on port : %d\n", port);
  return sockfd;
}


// SEND/ RECV function 

int send_all(int sockfd, size_t len, const char *reply) {
  ssize_t bytes = 0;
  int send_size = 4028;
  size_t total_sent = 0;
  // printf("file size: %ld\n", len);
  while (len) {
    bytes = send(sockfd, reply, len, 0);
    if (bytes == -1) {
      if (errno == EAGAIN || errno == EWOULDBLOCK) {
        // the send() buffer is full, retry it later
        fprintf(stderr, "EAGAIN, bytes sent : %ld\n", total_sent);
      }
    }
    total_sent += bytes;
    reply += bytes;
    len -= bytes;
  }
  // printf("total file size sent : %ld\n", total_sent);
  return 0;
}
