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

    event_loop->epoll_data_size = size;

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

    int op = flags == BZ_NONE ? EPOLL_CTL_ADD : EPOLL_CTL_MOD;

    if( epoll_ctl(efd,op,fd,&e) < 0 ){
        // if the operation failed, close the file descriptor
        close(fd);
        return ;
    }

}

void bz_remove_event(event_loop_t* event_loop, int fd, int flags){
    
}