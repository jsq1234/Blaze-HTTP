#include "../include/event_loop.h"
#include "../include/server.h"
#include "../include/http.h"
#include <stdlib.h>
#include <stdio.h>

#define RECV_SIZE 2048
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
            return -1;
    }
    int nfds = 0;
    struct sockaddr_in in_addr;
    socklen_t in_len;
    for(;;){
        nfds = epoll_wait(event->epollfd,event->events,MAX_EVENTS,-1);
        if( nfds == -1 ){
            perror("epoll_wait()");
            return -1;
        }

        for(int i=0; i<nfds; i++){
            if( event->events[i].data.fd == server.sockfd ) {
                // we recieved a new connection on the socket that the server is listening on
                // accept all the incoming new connections 
                
                while(1){

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
            if( event->events[i].events & EPOLLIN ){
                // client has send some data to the socket
                // recieve all the data at once
                int client_fd = event->events[i].data.fd;
                char buffer[RECV_SIZE];
                while(1){
                    char* ptr = buffer;
                    ssize_t bytes = recv(client_fd,ptr,RECV_SIZE-1,0);
                    if( bytes == 0 ){
                        // client has closed the connection
                        // close the client socket and remove from the epoll_loop_t
                        close(client_fd);
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
                
                http_t request;
                parse_request(buffer,strlen(buffer),&request);
                                
            
            }


    }
    return 0;
}


}











