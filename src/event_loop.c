#include "event_loop.h"
#include "client.h"
#include <errno.h>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <unistd.h>
#include <sys/sendfile.h>
#include <string.h>


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

int bz_add_event(int epfd, int fd, int flags){
    struct epoll_event e = {0};

    e.data.fd = fd;
    e.events = flags;

    int op = flags & BZ_NONE ? EPOLL_CTL_ADD : EPOLL_CTL_MOD;

    if( flags & BZ_READABLE ) e.events |= EPOLLIN;
    if( flags & BZ_WRITEABLE ) e.events |= EPOLLOUT;
    if( flags & BZ_EDGE_TRIG ) e.events |= EPOLLET;

    if( epoll_ctl(epfd,op,fd,&e) < 0 ){
        // if the operation failed, close the file descriptor
        perror("epoll_ctl()");
        return -1;
    }

    return 0;
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

            data_t* d = event.data.ptr;

            if( event.events & EPOLLERR ){

                fprintf(stderr, "Strange epoll error.\n");
                event_loop->handler.bz_handle_close_event(event_loop,d);

                continue;
            }
            if( event.events & EPOLLIN ){
        
                if( d->filefd < 0 ){
                    /*  
                        A negative filefd indicates that the data_t belongs to the server.
                        Thus, we have a new connection
                    */

                    event_loop->handler.bz_handle_new_connection(event_loop,d);

                }else{
                    event_loop->handler.bz_handle_read_event(event_loop,d);
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
                if( !(d->state & PENDING_REPLY) ){
                  continue;
                }
                
                event_loop->handler.bz_handle_write_event(event_loop, d);
                
            }

            if( d->state & CLOSED ){
                event_loop->handler.bz_handle_close_event(event_loop,d);
            }
        }
    }

    return 0;
}

void bz_read_event(event_loop_t* event_loop, data_t* d){

/*     if( d->state & PENDING_REPLY ){
        return ;
    } */

    /* Clear the previous state */

    int fd = d->fd;

    ssize_t bytesRcv = 0;
    size_t len = d->buff_size;

    int msg_completed = 0;
    int done = 0;
    
    /*  
        Note: A better way would be to parse as the messages arrive. 
        This will be implemented at a later date. For now, this code just 
        fills the client buffer with the messages that the server recieves.
    */
    for(;;){

        bytesRcv = read(fd, d->buff, len-1);
        if( bytesRcv == 0 ){
            /*  
                Client has sent a FIN packet and thus has closed the connection
                If we still have any data that we want to send, we should send it
                before closing our end.
            */

            d->state = DISCONNECTED | PENDING_REPLY;
            break;

        }
        if( bytesRcv == -1 ){
            if( errno == EAGAIN || errno == EWOULDBLOCK ){
                /* 
                    There is no message to read, try again later. This is 
                    rare because if this function was called then that means
                    this socket was readablea and thus had data that could be 
                    read.
                 */
                done = 1;
                d->state = CONNECTED; 
                break;
            }else{
                perror("read()");
                d->state = CLOSED;
                return ;
            }
        }

        d->buff[bytesRcv] = '\0';
        d->buff += bytesRcv;
        len -= bytesRcv;

        /* 
            For now, the server doesn't accept pipelined request and thus
            it is an assumption that the client will send one request message
            before sending another.
        */
        if( strcmp((const char*)d->buff - 4, "\r\n\r\n") == 0 ){
            msg_completed = 1;
        }
        
        
        /* To avoid some race conditions */
        if( msg_completed && done ){
            break;
        }
    }

/*     if( d->state & CONNECTED ){
        return ;
    }
 */

    /* Not totally correct, will change it once I add HTTP. */
    if( msg_completed )
        d->state |= PENDING_REPLY;

}

void bz_handle_new_connection(event_loop_t* event_loop, data_t* d){
    int epollfd = event_loop->epoll_data->epollfd;
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

        bz_connection_t* conn = malloc(sizeof(*conn));
        
        conn->sa = conn_addr;
        conn->len = conn_len;

        data_t* dt = malloc(sizeof(*dt));

        dt->fd = connfd;
        dt->buff = malloc(1024);
        dt->buff_size = 1024;
        dt->filefd = 0;
        dt->state = CONNECTED;
        dt->f_size = 0;
        dt->offset = 0;

        conn->d = dt;
        

        if( bz_add_event(epollfd,connfd,BZ_ALL) < 0 ){
            free(dt->buff);
            free(dt);
            free(conn);
            close(connfd);
            continue;
        }
        
        event_loop->connections[connfd] = conn;
    }
}

void bz_write_event(event_loop_t* event_loop, data_t* d){

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
                    Close the connection.
                */
                d->state = CLOSED;
                return ;
            }
        }
        left -= bytesSnd;
    }

    if(!left){
        /*  
            If we have sent all of the message we will change the state
            to either closed or connected.  
            This also assumes keep-alive, if we are not having a keep
            alive connection, we need to mark the state as closed. 
            To be done later. 
        */
        d->state = d->state & DISCONNECTED ? CLOSED : CONNECTED;
    }else{
        /*  We could not send the file because the kernel buffer was full  */
        d->state |= PENDING_REPLY;
    }


}

/* 
    This function closes the socket descriptor and frees the resources
*/

void bz_close_event(event_loop_t* event_loop, data_t* d){
    
    if(!d){
        fprintf(stderr, "Null data sent in bz_close_event\n");
        return ;
    }
    
    int fd = d->fd;
    
    bz_connection_t* conn = event_loop->connections[fd];
    
    event_loop->connections[fd] = NULL;

    
    close(fd);

    free(d);
    free(conn);
}

/* To do: add a bz_shutdown_event() once I add timer support */