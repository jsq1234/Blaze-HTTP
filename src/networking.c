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

int bz_create_socket(const char* sock_type){
    int fd = 0;
    if( strcasecmp(sock_type, "tcp") == 0 ){
        fd = socket(AF_INET,SOCK_STREAM,0);
        if( fd < 0 ){
            exit(1);
        }
    }
    else if( strcasecmp(sock_type, "udp") == 0 ){
        fd = socket(AF_INET,SOCK_DGRAM,0);
        if( fd < 0){
            exit(1);
        }
    }
    else{
        fprintf(stderr,"protocol not recognized.\n");
        exit(1);
    }

    return fd;

}

void bz_bind_socket(int fd, struct sockaddr_in* sadr){
    if( bind(fd,(const struct sockaddr*)&sadr, sizeof(*sadr)) < 0 ){
        perror("bind()");
        exit(1);
    }
}

void bz_start_listening(int fd, int backlog){
    if( listen(fd, backlog) < 0 ){
        perror("listen()");
        fprintf(stderr, "Server could not start. Aborting...\n");
        exit(1);
    }
}

int bz_set_socket_nonblocking(int fd){
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
