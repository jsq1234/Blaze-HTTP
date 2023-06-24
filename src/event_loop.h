#ifndef EVENT_LOOP_HEADER
#define EVENT_LOOP_HEADER

#include <stdio.h>
#include <stdlib.h>
#include <sys/epoll.h>

#define BZ_NONE 1<<0
#define BZ_READABLE 1<<1
#define BZ_WRITEABLE 1<<2

struct event_loop_ds;

typedef struct event_loop_ds event_loop_t;

typedef void (*event_handler)(event_loop_t* event_loop, int fd, );

typedef struct epoll_struct{
    int epollfd;
    struct epoll_event* events;
} bz_epoll_t;

typedef struct event_action_ds {
    void (*accept_connection)(event_loop_t* event_loop,int fd);
    void (*delete_connection)(event_loop_t* event_loop,int fd);
    void (*write_message)(event_loop_t* event_loop, int fd);
    void (*read_message)(event_loop_t* event_loop, int fd);
} event_action_t;

struct event_loop_ds{
    bz_epoll_t* epoll_data;
    int epoll_data_size;
    int maxfd;
    int flags;
    event_action_t actions;
};


event_loop_t* bz_create_event_loop(size_t size);
void bz_add_event(event_loop_t* event_loop, int op, int flags);
void bz_delete_event(event_loop_t* event_loop, int op, int flags);
#endif