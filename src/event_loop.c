#include "event_loop.h"
#include "client.h"
#include <errno.h>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <unistd.h>
#include <sys/sendfile.h>

static int bz_epoll_init(event_loop_t *event_loop, size_t size){

    bz_epoll_t* p = malloc(sizeof(*p));

    if( p == NULL ){
        perror("Couldn't create bz_epoll_t\n");
        return -1;
    }

    if( (p->epollfd = epoll_create1(0)) < 0 ){
        perror("epoll_create1\n");
        return -1;
    }

    p->events = malloc(sizeof(struct epoll_event)*size);

    if( p->events == NULL ){
        perror("Couldn't allocate bz_epoll_t.eventsn\n");
        free(p);
        return -1;
    }

    p->max_size = size;

    event_loop->epoll_data = p;

    return 0;
}

event_loop_t* bz_create_event_loop(size_t size){

    event_loop_t* event_loop = malloc(sizeof(*event_loop));
    
    if( bz_epoll_init(event_loop, size) < 0 ){
        free(event_loop);
        return NULL;
    }

    event_loop->flags = 0;
    event_loop->maxfd = size;

    /* 
        Event handlers that are called when a specific event arrives.
        TO_DO : Support something generic for future use.
    */

    event_loop->handler.bz_handle_new_connection = bz_handle_new_connection;
    event_loop->handler.bz_handle_read_event = bz_read_event;
    event_loop->handler.bz_handle_write_event = bz_write_event;
    event_loop->handler.bz_handle_close_event = bz_close_event;

    /* TO DO : More members need initialization. */
    return event_loop;
}

void bz_add_event(int epfd, int fd, int flags){
    struct epoll_event e = {0};

    e.data.fd = fd;
    e.events = flags;

    int op = flags & BZ_NONE ? EPOLL_CTL_ADD : EPOLL_CTL_MOD;

    if( flags & BZ_READABLE ) e.events |= EPOLLIN;
    if( flags & BZ_WRITEABLE ) e.events |= EPOLLOUT;
    if( flags & BZ_EDGE_TRIG ) e.events |= EPOLLET;

    if( epoll_ctl(epfd,op,fd,&e) < 0 ){
        // if the operation failed, close the file descriptor
        close(fd);
    }
}


int run_event_loop(event_loop_t* event_loop){
    for(;;){
        if( bz_process_events(event_loop) <= 0 ){
            /* TO_DO : Perform clean up action */
        }
    }
    return 0;
}

int bz_process_events(event_loop_t* event_loop){

    bz_epoll_t* ev = event_loop->epoll_data;

    int nfds = epoll_wait(ev->epollfd,ev->events,ev->max_size,-1);

    if( nfds < 0 ){
        perror("epoll_wait\n");
        return -1;

    }else if ( nfds == 0 ){

        printf("Epoll timed out...\n");
        return 0;

    }else{

        int epoll_fd = event_loop->epoll_data->epollfd;

        for(int i=0; i<nfds; i++){

            struct epoll_event event = ev->events[i];

            if( event.events & EPOLLERR ){
                perror("epoll_err");
                return -1;
            }
            if( event.events & EPOLLIN ){
                data_t* d = (data_t*)event.data.ptr;

                if( d->filefd < 0 ){
                    /*  
                        A negative filefd indicates that the data belongs to the server.
                        Thus, we have a new connection
                     */
                    event_loop->handler.bz_handle_new_connection(epoll_fd,d);

                }else{
                    event_loop->handler.bz_handle_read_event(epoll_fd,d);
                }
            }
            if( event.events & EPOLLOUT ){
                data_t* d = (data_t*)event.data.ptr;
                /*  
                    If we have received the message from the client,
                    only then we will send the response.
                    This is an alternative to setting EPOLLOUT flag 
                    with epoll_ctl. I am not sure if this actually 
                    increases the performace by reducing epoll_ctl system call.
                */
                if( d->state == MSG_RECVD ){
                    event_loop->handler.bz_handle_write_event(epoll_fd, d);
                }
            }
        }
    }

    return 0;
}

void bz_read_event(int epollfd, data_t* d){
    /*  negative filefd indicates that read event occured on 
        server socket. This is a new connection event which will
        be handled by another function. */

    if( d->filefd < 0 ){
        bz_handle_new_connection(epollfd,d);
        return ;
    }

    int fd = d->fd;

    ssize_t bytesRcv = 0;
    size_t len = d->buff_size;
    for(;;){
        bytesRcv = read(fd, d->buff, len);
        if( bytesRcv == 0 ){
            /*  Client has sent a FIN packet and thus has closed the connection
                If we still have any data that we want to send, we should send it
                before closing our end.
            */

        }
        if( bytesRcv == -1 ){
            if( errno == EAGAIN || errno == EWOULDBLOCK ){
                /* We have received everything that we could. */
                d->state = PENDING_REPLY;
                break;
            }
        }

        d->buff[len] = '\0';
        d->buff += len;
        len -= bytesRcv;
    }

}

void bz_handle_new_connection(int epoll_fd, data_t* d){
    int sockfd = d->fd;
    
    struct sockaddr_in conn_addr;
    socklen_t conn_len;
    int connfd;

    for(;;){
        connfd = accept(sockfd,(struct sockaddr*)&conn_addr,&conn_len);
        if( connfd == -1 ){
            if( errno == EAGAIN || errno == EWOULDBLOCK ){
                /* We have processed all the connections. */
                return ;
            }
        }

#ifdef DBG
        printf("New client connected : %d\n", connfd);
#endif

        /* Now we create a new client data structure */

        client_connection_t* conn = malloc(sizeof(*conn));
        conn->sa = conn_addr;
        conn->len = conn_len;
        /* To do -> Create a buffer struct instead */
        conn->d.buff = malloc(1024);
        conn->d.fd = connfd;
        conn->d.filefd = 0;
        conn->d.state = CONNECTED;
        conn->d.f_size = 0;
        conn->d.offset = 0;
        
        bz_add_event(epoll_fd,connfd,BZ_ALL);
    }
}

void bz_write_event(int epollfd, data_t* d){

    int fd = d->fd;
    int filefd = d->filefd;
    off_t file_size = d->f_size;
    off_t offset = d->offset;
    off_t left = file_size - offset;

    ssize_t bytesSnd = 0;

    while(left){
        bytesSnd = sendfile(fd, filefd, &offset, left);
        if( bytesSnd == -1 ){
            if( errno == EAGAIN || errno == EWOULDBLOCK ){
                /*  This indicates that that kernel buffers are full
                    Mark this file descriptor as pending and retry again  */
                    d->state = PENDING_REPLY;
                    break;
            }else{
                perror("sendfile()");
                /* 
                    Close the connection in case of any error.
                    Possible scenario -> Connection Reset/Broken pipe
                 */
                d->state = CLOSED;
                break;
            }
        }
        left -= bytesSnd;
    }

    if( d->state == PENDING_REPLY ){
        d->offset = offset;
        return ;
    }

    d->state = REPLY_SENT;

}


/* 
    struct server_ds;
    typedef server_ds server_t;

    struct server_ds{
        int sockfd;
        struct sockaddr_in sa;
        socklen_t len;
        data_t dt;
        event_loop_t* event_loop;
    };

 */