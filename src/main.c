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
#include <sys/sendfile.h>
#include "../include/utils.h"
#include "../include/http.h"
#include "../include/logger.h"

// SERVER HEADER
#define RECV_SIZE 2048

logger_t logger;

typedef struct server_ds {
  int sockfd;
  struct sockaddr_in info;

  unsigned char *reply;    // pointer to the message that needs to be sent
  unsigned char *send_ptr; /* pointer to the first element in reply
                              that needs to be sent.
                            */
  size_t left;             // length of the message left to send
} server_t;

int mcount = 0;
int fcount = 0;
server_t server;

void init_server(uint16_t port);
int make_socket_nonblocking(int fd);
ssize_t send_all(int sockfd, size_t len, const unsigned char *reply, int *client_state);
int generate_response(http_t *request, int sockfd);
size_t OK_reply(FILE *fptr, const char *file_path,long f_size, http_t *request);
ssize_t send_file(int sockfd, int file_fd, size_t len);
// =================================================================

// EPOLL HEADER

#define MAX_EVENTS 10000

typedef struct event_loop {
  int epollfd;
  struct epoll_event ev;
  struct epoll_event events[MAX_EVENTS];
} event_loop_t;

int init_loop(event_loop_t *ev_ds);
int run_event_loop(event_loop_t *event);

int main(int argc, char **argv) {
  if (argc < 2) {
    fprintf(stderr, "Usage : %s [port]\n", argv[0]);
    exit(1);
  }

  uint16_t port = atoi(argv[1]);
  init_server(port);
  event_loop_t event;
  init_loop(&event);

  if( init_logger(&logger,"log.txt") < 0 ){
      fprintf(stderr,"logger initialization failed.\n");
  }

  run_event_loop(&event);
}

void init_server(uint16_t port) {

  int sockfd = socket(AF_INET, SOCK_STREAM, 0);
  if (sockfd < 0) {
    perror("socket()");
    exit(1);
  }
  memset(&server, 0, sizeof(server));

  server.info.sin_family = AF_INET;
  server.info.sin_port = htons(port);
  server.info.sin_addr.s_addr = INADDR_ANY;

  if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &(int){1},
                 sizeof(int)) < 0) {
    perror("setsockopt()");
    exit(1);
  }

  if (bind(sockfd, (const struct sockaddr *)&server.info, sizeof(server.info)) <
      0) {
    perror("bind()");
    exit(1);
  }

  if (make_socket_nonblocking(sockfd) == -1) {
    exit(1);
  }

  server.sockfd = sockfd;

  if (listen(sockfd, 10000) < 0) {
    perror("listen()");
    exit(1);
  }
  printf("server is listening...\n");
  printf("server fd : %d\n", server.sockfd);
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


// EPOLL STARTS ->

int init_loop(event_loop_t *event) {
  event->epollfd = epoll_create1(0);
  if (event->epollfd < 0) {
    perror("epoll_create()");
    exit(1);
  }
  memset(&event->ev, 0, sizeof(event->ev));
  memset(&event->events, 0, sizeof(event->events));
  return 0;
}

int run_event_loop(event_loop_t *event) {
  event->ev.events = EPOLLIN;
  event->ev.data.fd = server.sockfd;

  if (epoll_ctl(event->epollfd, EPOLL_CTL_ADD, server.sockfd, &event->ev)) {
    perror("epoll_ctl: listen sock");
    exit(1);
  }

  int nfds = 0;
  struct sockaddr_in in_addr;
  socklen_t in_len;

  for (;;) {

#ifdef DBG
     printf("blocking on epoll_wait()\n");
#endif

    nfds = epoll_wait(event->epollfd, event->events, MAX_EVENTS, 5000);

    if( nfds == 0 ){
        printf("timeout! Closing..\n");
        close(server.sockfd);
        return 0;
    }

#ifdef DBG
    printf("waking from epoll_wait()\n");
#endif

    if (nfds == -1) {
      perror("epoll_wait()");
      return -1;
    }

    for (int i = 0; i < nfds; i++) {
      char buffer[RECV_SIZE];
      int client_closed = 0;

      if (event->events[i].events == EPOLLHUP) {
        fprintf(stderr, "got EPOLLUP\n");
      }
      if (event->events[i].events == EPOLLERR) {
        fprintf(stderr, "got EPOLLERR\n");
      }
      if (event->events[i].events & EPOLLIN) {

        if (event->events[i].data.fd == server.sockfd) {
          // we recieved a new connection on the socket that the server is
          // listening on accept all the incoming new connections
          while (1) {
            memset(&in_addr, 0, sizeof(in_addr));
            in_len = sizeof(in_addr);

            int connfd =
                accept(server.sockfd, (struct sockaddr *)&in_addr, &in_len);

            if (connfd == -1) {
              if (errno == EAGAIN || errno == EWOULDBLOCK) {
                // we have accepted all the connections that we could
                // no more connection, break out of this loop
                break;
              }
              perror("accept()");
              break;
            }
            //char log_msg[150];
            
            //snprintf(log_msg,150, "New client %d connection!", connfd);
            
            //log_message(&logger,log_msg);
            #ifdef DBG
            
            printf(GREEN "New client %d connection!\n" RESET, connfd);


            #endif
            if (make_socket_nonblocking(connfd) == -1) {
              continue;
            }

            event->ev.events = EPOLLIN | EPOLLET;
            event->ev.data.fd = connfd;

            if (epoll_ctl(event->epollfd, EPOLL_CTL_ADD, connfd, &event->ev) <
                0) {
              fprintf(stderr, "epoll set insertion error: fd=%d", connfd);
              close(connfd);
              continue;
            }
          }
        } else {
          // client has send some data to the socket
          // recieve all the data at once
          int client_fd = event->events[i].data.fd;
          int count = 0;
          while (1) {
            char *ptr = buffer;
            count++;
            ssize_t bytes = recv(client_fd, ptr, RECV_SIZE - 1, 0);
            if (bytes == 0) {
              // client has closed the connection
              // close the client socket
              // if we have still some data left to send, then we can still send
              // it it's best to close the socket AFTER we have sent the message
              #ifdef DBG

              printf(RED "Client %d closed connection\n" RESET, client_fd);
                
              #endif
              // we indicate that the client has closed it's connection by
              // setting the client_closed variable to 1.
              client_closed = 1;

              break;

            } else if (bytes == -1) {
              if (errno == EAGAIN || errno == EWOULDBLOCK) {
                // we have read all we could
                // break from the while loop

                break;
              }
              perror("recv()");
              break;
            } else {
              ptr[bytes] = '\0';
              
              ptr += bytes;
              // The following is based on the assumption that
              // HTTP pipelining is disabled. This server doesn't
              // hanlde pipelined requests and assumes that the client sends
              // one request at once.

              if (strncmp(ptr - 4, "\r\n\r\n", 4) == 0) {
             //printf("message recieved\n");
                break;
              }
            }
          }
          
          //printf("recieved message %d : \n%s", client_fd, buffer);

#ifdef DBG
          if (client_closed) {
            printf("Client has closed his end, nothing to receive.\n");
          } else
            printf("recieved a message from client %d\n", client_fd);
#endif
          if (!client_closed) {
            http_t request;

            if (parse_request(buffer, strlen(buffer), &request) ==
                NOT_IMPLEMENTED) {
              // the request is NOT a GET request;
              // send a NOT_IMPLEMENTED REPLY
#ifdef DBG
              printf("sending not_implemented reply\n");
#endif
              server.send_ptr = (unsigned char*)not_implemented_reply;
              server.left = strlen(not_implemented_reply);

            } else if (generate_response(&request, client_fd) == NOT_FOUND) {

#ifdef DBG
              printf("sending not_found reply\n");
#endif
              server.send_ptr = (unsigned char*)not_found_reply;
              server.left = strlen(not_found_reply);
            }
          }

          event->events[i].events = EPOLLOUT | EPOLLET;

          if (epoll_ctl(event->epollfd, EPOLL_CTL_MOD, client_fd,
                        &event->events[i])) {
            perror("epoll_ctl");
          }
        }
      }
      if (event->events[i].events & EPOLLOUT) {

        int client_fd = event->events[i].data.fd;

        ssize_t bytesSnd =
            send_all(client_fd, server.left, server.send_ptr, &client_closed);

        if (bytesSnd < 0) {
#ifdef DBG
          printf("Buffer is full, continuing...\n");
#endif
          server.left += bytesSnd;
          server.send_ptr += -bytesSnd;
          continue;
        }

        // printf("sent message: \n%s", server.reply);
        server.left -= bytesSnd;
      
        if (server.reply != NULL && server.left <= 0) {
        //printf("message sent to client\n");
#ifdef DBG
          printf("Sent message to the client %d\n", client_fd);
#endif
          fcount++;
          free(server.reply);
          server.reply = NULL;
          server.send_ptr = NULL;
        }

        if (client_closed) {

#ifdef DBG
          printf("Closing the client\n");
#endif

          close(client_fd);
          continue;
        }

        event->events[i].events = EPOLLIN | EPOLLET;

        if (epoll_ctl(event->epollfd, EPOLL_CTL_MOD, event->events[i].data.fd,
                      &event->events[i])) {
          perror("epoll_ctl()");
        }
      }
    }
  }
}

// send len bytes of a file identified by file_fd to sockfd 
ssize_t send_file(int sockfd, int file_fd, size_t len){
    
    ssize_t bytes = 0;
    size_t total_sent = 0;

    while(len){
        bytes = sendfile(sockfd,file_fd,NULL,len);
        if( bytes == -1 ){
            if( errno == EAGAIN || errno == EWOULDBLOCK ){
                // the kernel buffer is full, wait for it to free up a little
                // send later 

                return -total_sent;
            }else{
                perror("send()");
            }
        }

        total_sent += bytes;
        len -= bytes;
    }

    return total_sent;
}
ssize_t send_all(int sockfd, size_t len, const unsigned char *reply, int *client_state) {

  ssize_t bytes = 0;
  size_t total_sent = 0;

  while (len) {
    bytes = send(sockfd, reply, len, MSG_NOSIGNAL);

#ifdef DBG
        printf("bytes sent : %ld\n", bytes);
#endif
    if (bytes == -1) {
      if (errno == EAGAIN || errno == EWOULDBLOCK) {
        // the send() buffer is full, retry it later

#ifdef DBG
        printf("Buffer full, wait...\n");
#endif
        return -total_sent;
      } else {
        perror("send()");
        *client_state = 1;
        //printf("errno : %d\n", errno);
        break;
      }
    }
    total_sent += bytes;
    reply += bytes;
    len -= bytes;
  }
  // printf("total file size sent : %ld\n", total_sent);
  return total_sent;
}

int generate_response(http_t *request, int sockfd) {
  char *file_path = "";

  if (strcmp(request->url, "/") == 0) {
    file_path = "index.html";
  } else {
    file_path = &request->url[1];
  }

  FILE *fptr = fopen(file_path, "r");

  if (fptr == NULL) {
    // handle file not found error
    // return -1 in case the local send() socket buffer is full
    return NOT_FOUND;
  }

  struct stat file_info;

  if (stat(file_path, &file_info) < 0) {
    perror("stat()");
    return -1;
  }

  long f_size = file_info.st_size;

  server.left = OK_reply(fptr, file_path, f_size, request);

  fclose(fptr);

  return server.left;

}


size_t OK_reply(FILE *fptr, const char *file_path, long f_size, http_t *request) {

  char response_header[512];
  response_header[0] = '\0';

  const char *status_code = "200";
  const char *status_msg = "OK";
  char content_type[50] = "";

  get_content_type(content_type, file_path);

  snprintf(response_header, 512,
           "%s %s %s\r\n"
           "Content-Type: %s\r\n"
           "Conntection: keep-alive\r\n"
           "Content-Length: %ld\r\n\r\n",
           request->version, status_code, status_msg, content_type, f_size);
  //+1 for null terminating character
  int res_len = strlen(response_header);
  size_t reply_size = res_len + f_size + 1;

  if (reply_size >= LARGE_FILE) {
    // DO NOT FORGET TO FREE AFTER SENDING REPLY TO THE CLIENT
    server.reply = calloc(reply_size,1);
    
    if( server.reply ==  NULL ){
      fprintf(stderr, "Couldn't allocate memory\n");
      return -1;
    }

    
    strcpy((char *)server.reply, response_header);

    size_t rdbytes = 0;
    
    rdbytes = read_large_file(fptr, f_size, server.reply + res_len);

    server.send_ptr = server.reply;
  
  } else {
    // DO NOT FORGET TO FREE AFTER SENDING THE REPLY TO THE CLIENT!
    server.reply = calloc(reply_size,1);
    mcount++;


    if( server.reply ==  NULL ){
      fprintf(stderr, "Couldn't allocate memory\n");
    }
    strcpy((char *)server.reply, response_header);
    
    size_t rdBytes = 0;
    
    rdBytes = read_file(fptr, f_size, server.reply + res_len);

    server.send_ptr = server.reply;
  }

  return reply_size;
}
