#include "event_loop.h"
#include <sys/epoll.h>
#include <sys/socket.h>
#include <unistd.h>

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

    event_loop->max_size = size;

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

    return event_loop;
}

void bz_add_event(event_loop_t *event_loop, int fd, int flags){
    int efd = event_loop->epoll_data->epollfd;

    struct epoll_event e = {0};

    e.data.fd = fd;
    e.events = flags;

    int op = flags & BZ_NONE ? EPOLL_CTL_ADD : EPOLL_CTL_MOD;

    if( flags & BZ_READABLE ) e.events |= EPOLLIN;
    if( flags & BZ_WRITEABLE ) e.events |= EPOLLOUT;
    if( flags & BZ_EDGE_TRIG ) e.events |= EPOLLET;

    if( epoll_ctl(efd,op,fd,&e) < 0 ){
        // if the operation failed, close the file descriptor
        close(fd);
    }
}

void bz_delete_event(event_loop_t *event_loop, int fd, int del_mask){
    bz_epoll_t* epl = event_loop->epoll_data;
    struct epoll_event e = {0};
    // to do later
}


int bz_handle_events(event_loop_t* event_loop){

    bz_epoll_t* ev = event_loop->epoll_data;
    
    int nfds = epoll_wait(ev->epollfd,ev->events,ev->max_size,-1);

    if( nfds < 0 ){
        perror("epoll_wait\n");
        return -1;
    }else if ( nfds == 0 ){
        printf("Epoll timed out...\n");
        return 0;
    }else{
        for(int i=0; i<nfds; i++){
            struct epoll_event event = ev->events[i];
            if( event.events & EPOLLERR){
                perror("epoll_err");
                return -1;
            }
            if( event.events & EPOLLIN ){
                data_t* d = (data_t*)event.data.ptr;
            }
            if( event.events & EPOLLOUT ){
                data_t* d = (data_t*)event.data.ptr;

            }
        }
    }

    return 0;
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