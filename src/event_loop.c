#include "event_loop.h"

int bz_epoll_init(bz_epoll_t* ep, size_t size){
    ep->epollfd = epoll_create1(0);
    if( ep->epollfd == -1 ){
        perror("epoll_create\n");
        return -1;
    }

    ep->events = malloc(sizeof(struct epoll_event)*size);

    if( ep->events == NULL ){
        perror("Couldn't allocate memory for ep->events\n");
        return -1;
    }

    return 0;
}

