
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/epoll.h>
#include <arpa/inet.h>
#include <errno.h>
#include <unistd.h>
#include <string.h>
#include <stdint.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "../include/utils.h"

#define MOVE(p,l,n) p += n; l -= n;
#define TRAVERSE_TILL(p,ch) while( *p != '\0' && *p != ch ) ++p; 
#define CHECK_EOF(l,p) if( l == 4 && strncmp(p,"\r\n\r\n", 4) == 0 ){ return 0; }



// HTTP HEADER 
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

const char* not_found_reply = "HTTP/1.1 404 Not Found\r\n" "Content-Length: 0\r\n\r\n";
const char* not_implemented_reply = "HTTP/1.1 203 Not implemented\r\n" "Content-Length: 0\r\n\r\n";


int parse_request(const char *msg, size_t len, http_t *request);
// ----------------------------------------------------------------
// ----------------------------------------------------------------


// SERVER HEADER
#define RECV_SIZE 2048
typedef struct server_ds {
    int sockfd;
    struct sockaddr_in info;
    char* reply; 
} server_t;


server_t server;

void init_server(uint16_t port);
int make_socket_nonblocking(int fd);
int send_all(int sockfd, size_t len, const char *reply);
int send_response(http_t *request, int sockfd);
size_t OK_reply(FILE *fptr, const char *file_path, http_t *request);
// =================================================================

// EPOLL HEADER 

#define MAX_EVENTS 100

typedef struct event_loop {
    int epollfd;
    struct epoll_event ev;
    struct epoll_event events[MAX_EVENTS];
} event_loop_t;

int init_loop(event_loop_t* ev_ds);
int run_event_loop(event_loop_t* event);

int main(int argc, char** argv){
    if( argc < 2 ){
        fprintf(stderr,"Usage : %s [port]\n", argv[0]);
        exit(1);
    }

    uint16_t port = atoi(argv[1]);
    init_server(port);
    event_loop_t event;
    init_loop(&event);

    run_event_loop(&event);

    
}

void init_server(uint16_t port) {

    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        perror("socket()");
        exit(1);
    }
    memset(&server,0,sizeof(server));

    server.info.sin_family = AF_INET;
    server.info.sin_port = htons(port);
    server.info.sin_addr.s_addr = INADDR_ANY;

    if( setsockopt(sockfd,SOL_SOCKET,SO_REUSEADDR, &(int){1},sizeof(int)) < 0 ){
        perror("setsockopt()");
        exit(1);
    }

    if (bind(sockfd, (const struct sockaddr *)&server.info, sizeof(server.info)) <
            0) {
        perror("bind()");
        exit(1);
    }

    if( make_socket_nonblocking(sockfd) == -1 ){
        exit(1);
    }

    server.sockfd = sockfd;

    if (listen(sockfd, 100) < 0) {
        perror("listen()");
        exit(1);
    }
    printf("server is listening...\n");
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

int parse_request(const char *msg, size_t msg_len, http_t *request) {
  const char *ptr = msg;
  size_t left = msg_len;
  
  if (strncmp(ptr, "GET", strlen("GET")) == 0) {
    strcpy(request->method, "GET");
    MOVE(ptr,left,4)
  } else {
    return -1;
  }
  const char *itr = ptr;
  TRAVERSE_TILL(itr, ' ')
  int len = itr - ptr;

  strncpy(request->url, ptr, len);
  request->url[len] = '\0';

  MOVE(ptr,left,len+1) // +1 for the space 
  itr = ptr;

  TRAVERSE_TILL(itr, '\r')
  len = itr - ptr;

  strncpy(request->version, ptr, len);
  request->version[len] = '\0';

  return 0;
  // to be implemented in the future
  // for now, only parses the first line, i.e the request line

  MOVE(ptr,left,len)
  // if we have reached EOF (\r\n\r\n) the return 
  CHECK_EOF(left,ptr)
  //move by 2 to go the next line 

  return 0;
}


// EPOLL STARTS -> 

int init_loop(event_loop_t* event){
    event->epollfd = epoll_create1(0);
    if( event->epollfd < 0 ){
        perror("epoll_create()");
        exit(1);
    }
    memset(&event->ev,0,sizeof(event->ev));
    memset(&event->events,0,sizeof(event->events));
    return 0;  
}


int run_event_loop(event_loop_t* event){
    event->ev.events = EPOLLIN;
    event->ev.data.fd = server.sockfd;

    if(epoll_ctl(event->epollfd, EPOLL_CTL_ADD, server.sockfd, &event->ev)){
            perror("epoll_ctl: listen sock");
            exit(1);
    }

    int nfds = 0;
    struct sockaddr_in in_addr;
    socklen_t in_len;

    for(;;){
        // printf("before wait\n");
        nfds = epoll_wait(event->epollfd,event->events,MAX_EVENTS,-1);
         //printf("after wait\n");

        if( nfds == -1 ){
            perror("epoll_wait()");
            return -1;
        }

        for(int i=0; i<nfds; i++){
            if( event->events[i].events == EPOLLHUP ){
                fprintf(stderr, "got EPOLLUP\n");
            }
            if( event->events[i].events == EPOLLERR ){
                fprintf(stderr,"got EPOLLERR\n");
            }
            if( event->events[i].data.fd == server.sockfd && event->events[i].events & EPOLLIN ) {
                // we recieved a new connection on the socket that the server is listening on
                // accept all the incoming new connections 
                while(1){
                    memset(&in_addr,0,sizeof(in_addr));
                    int connfd = accept(server.sockfd,(struct sockaddr*)&in_addr,&in_len);

                    if( connfd == -1 ){
                        if( errno == EAGAIN || errno == EWOULDBLOCK ){
                            // we have accepted all the connections that we could
                            // no more connection, break out of this loop
                            break;
                        }
                        perror("accept()");
                        break;
                    }
                    
                    if( make_socket_nonblocking(connfd) == -1 ){
                        continue;
                    }
                    
                    event->ev.events = EPOLLIN | EPOLLET;
                    event->ev.data.fd = connfd;
                    
                    if(epoll_ctl(event->epollfd,EPOLL_CTL_ADD,connfd,&event->ev) < 0 ){
                        fprintf(stderr,"epoll set insertion error: fd=%d", connfd);
                        close(connfd);
                        continue;
                    }
                }
                
            }
            else if( event->events[i].events & EPOLLIN ){
                // client has send some data to the socket
                // recieve all the data at once
                int client_fd = event->events[i].data.fd;
                char buffer[RECV_SIZE];
                int client_closed = 0;

                while(1){
                    char* ptr = buffer;
                    ssize_t bytes = recv(client_fd,ptr,RECV_SIZE-1,0);
                    if( bytes == 0 ){
                        // client has closed the connection
                        // close the client socket and remove from the epoll_loop_t
                        printf("Client %d closed connection\n", client_fd);
                        client_closed = 1;
                        close(client_fd);
                        break;
                    }else if( bytes == -1 ){
                        if( errno == EAGAIN || errno == EWOULDBLOCK ){
                            // we have read all we could
                            // break from the while loop
                            break;
                        }
                        perror("recv()");
                        break;
                    }else{
                        ptr[bytes] = '\0';
                        ptr += bytes;
                    }

                }
                // if client has not closed the socket
                // only then send the reply 
                printf("client closed var : %d\n", client_closed);
                if(!client_closed){ 
                    http_t request;
                    // parse_request returns -1 when the request is something other than 
                    // GET request. In that case we send the default NOT implemented message 
                    // to the client
                    if( parse_request(buffer,strlen(buffer),&request) == -1 ){
                        // send_all returns -1 only when send() call returns with EAGAIN or EWOULDBLOCK
                        // this indicates that the send buffers are full
                        // retry it again later
                        if( send_all(client_fd,strlen(not_implemented_reply),not_implemented_reply) == -1 ){
                            // for now continue
                            // to be implemented later
                            continue;
                        }
                    }

                    printf("sending response...\n");
                    if ( send_response(&request,client_fd) == -1 ){
                        fprintf(stderr, "send returned -1\n");
                    }
                    printf("sent OK reply\n");
                    // free the server reply which was created using malloc
                    if( server.reply != NULL ){
                        free(server.reply);
                        server.reply = NULL;
                    }
                    
                }
                
                                
            
            }


    }
}

}
int send_all(int sockfd, size_t len, const char *reply) {
  ssize_t bytes = 0;
  while (len) {
    bytes = send(sockfd, reply, len, 0);
    if (bytes == -1) {
      if (errno == EAGAIN || errno == EWOULDBLOCK) {
        // the send() buffer is full, retry it later
        return -1;
      }
    }
    reply += bytes;
    len -= bytes;
  }
  return 0;
}


int send_response(http_t *request, int sockfd) {
  char *file_path = "";

  if (strcmp(request->url, "/") == 0) {
    file_path = "index.html";
  } else {
    file_path = &request->url[1];
  }
  FILE *fptr = fopen(file_path, "r");
  if (fptr == NULL) {
    printf("NULL?\n");
    // handle file not found error
    // return -1 in case the local send() socket buffer is full
    return send_all(sockfd, strlen(not_found_reply), not_found_reply);
  }
printf("sending OK reply\n");
  size_t reply_len = OK_reply(fptr, file_path, request);
    return send_all(sockfd, reply_len, server.reply);

}


size_t OK_reply(FILE *fptr, const char *file_path, http_t *request) {

  struct stat file_info;
  if (stat(file_path, &file_info) < 0) {
    perror("stat()");
    return -1;
  }

  long f_size = file_info.st_size;

  char response_header[512];
  response_header[0] = '\0';

  const char *status_code = "200";
  const char *status_msg = "OK";
  char content_type[50] = "";

  get_content_type(content_type, file_path, strlen(file_path));
    printf("content-type : %s\n",content_type);
  snprintf(response_header, 512,
           "%s %s %s\r\n"
           "Content-Type: %s\r\n"
           "Conntection: keep-alive\r\n"
           "Content-Length: %ld\r\n\r\n",
           request->version, status_code, status_msg, content_type, f_size);
  // +1 for null terminating character
  size_t reply_size = strlen(response_header) + f_size + 1;

  // DO NOT FORGET TO FREE AFTER SENDING THE REPLY TO THE CLIENT!
  server.reply = (char *)malloc(sizeof(char) * reply_size);

    strcpy(server.reply,response_header);
  char *ptr = server.reply + strlen(response_header);

  read_text_file(fptr, f_size, ptr);

  return reply_size;
}
