#ifndef EVENT_LOOP_HEADER
#define EVENT_LOOP_HEADER

#include <sys/epoll.h>
#include <sys/socket.h>
#include <unistd.h>
#include <string.h>

#define MAX_EVENTS 100
typedef struct event_loop {
    int epollfd;
    struct epoll_event ev;
    struct epoll_event events[MAX_EVENTS];
} event_loop_t;

int init_loop(event_loop_t* ev_ds);
#endif
