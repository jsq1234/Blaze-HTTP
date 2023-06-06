#include "../include/util.h"

char buffer[BUF_SIZE];

int init_server(uint16_t port) {

  int sockfd;
  struct sockaddr_in serv_adr;

  if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
    perror("socket()");
    exit(1);
  }

  int y = 1;
  if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &y,
                 sizeof(y)) == -1) {
    perror("setsockopt()");
  }

  memset(&serv_adr, 0, sizeof(serv_adr));

  serv_adr.sin_family = AF_INET;
  serv_adr.sin_port = htons(port);
  serv_adr.sin_addr.s_addr = INADDR_ANY;

  if (bind(sockfd, (const struct sockaddr *)&serv_adr, sizeof(serv_adr)) < 0) {
    perror("bind()");
    exit(1);
  }

  if (listen(sockfd, 100) == -1) {
    perror("listen()");
    exit(1);
  }

  memset(buffer, 0, sizeof(buffer));
  return sockfd;
}

int set_socket_nonblocking(int sockfd) {
  int flags;
  flags = fcntl(sockfd, F_GETFL, 0);
  if (flags == -1) {
    perror("fcntl()");
    return -1;
  }

  flags |= O_NONBLOCK;

  if (fcntl(sockfd, F_SETFL, flags) == -1) {
    perror("fcntl()");
    return -1;
  }
  return 0;
}

int init_epoll() {
  int fd = epoll_create1(0);
  if (fd < 0) {
    perror("epoll_create1()");
    exit(1);
  }
  return fd;
}

void add_event(int epoll_fd, int sockfd, struct epoll_event *event,
               uint32_t flags) {
  // make file descriptor sockfd non-blocking and add file descriptor sockfd to
  // interest list
  if (set_socket_nonblocking(sockfd) == -1) {
    exit(1);
  }
  event->events = flags;
  event->data.fd = sockfd;
  if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, sockfd, event) < 0) {
    perror("epoll_ctl()");
    exit(1);
  }
}

int concat(char *restrict dst, char *restrict src, size_t max_size) {
  // concatenate n bytes of src to dst and null terminate dst
  // also checks for buffer overflow
  size_t dst_len = strlen(dst);
  size_t src_len = strlen(src);
  if (dst_len + src_len + 1 > max_size) { // 1 byte for null character
    return -1;
  }
  memcpy(dst + dst_len, src, src_len);
  dst[dst_len + src_len] = '\0';
  return 0;
}

int send_all(int sockfd, char *msg) {
  size_t len = strlen(msg);
  size_t left = len;
  const char *ptr = msg;
  while (left) {
    ssize_t bytes = send(sockfd, ptr, left, 0);
    if (bytes < 0) {
      perror("send()");
      return -1;
    }
    ptr += bytes;
    left -= bytes;
  }
  printf("sent : %s\n",msg);
  return 0;
}

