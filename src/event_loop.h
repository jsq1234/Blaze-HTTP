#ifndef EVENT_LOOP_HEADER
#define EVENT_LOOP_HEADER

#include <stdio.h>
#include <stdlib.h>
#include <sys/epoll.h>

struct event_loop_ds;
typedef struct event_loop_ds event_loop_t;

typedef void (*event_handler)(event_loop_t* event_loop);

typedef struct epoll_struct{
    int epollfd;
    struct epoll_event* events;
} bz_epoll_t;

struct event_loop_ds{
    bz_epoll_t* epoll_data;
    int epoll_data_size;
    int maxfd;
    int flags;

} event_loop_t;


#endif