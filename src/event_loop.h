#ifndef EVENT_LOOP_HEADER
#define EVENT_LOOP_HEADER

#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/epoll.h>
#include <sys/socket.h>
#include "client.h"

#define BZ_NONE 1<<0
#define BZ_READABLE 1<<1
#define BZ_WRITEABLE 1<<2
#define BZ_EDGE_TRIG 1<<3
#define BZ_ALL (BZ_READABLE|BZ_WRITEABLE|BZ_NONE|BZ_EDGE_TRIG)



struct event_loop_ds;

typedef struct event_loop_ds event_loop_t;

typedef void bz_event_handler_t(int e, data_t* d);

typedef struct epoll_struct{
    int epollfd;
    struct epoll_event* events;
    int max_size;
} bz_epoll_t;


struct event_loop_ds{
    bz_epoll_t* epoll_data;
    int maxfd;
    int flags;
    bz_event_handler_t* event_handler;
};



/* To be put later in another header file  */

event_loop_t* bz_create_event_loop(size_t size);
void bz_add_event(int epfd, int fd,int flags);
void bz_delete_event(event_loop_t* event_loop, int fd, int del_mask);

void bz_handle_new_connection(int epoll_fd, data_t* d);
void bz_handle_read_event(int epoll_fd,data_t* d);
void bz_handle_write_event(int epoll_fd, data_t* d);
void bz_handle_close_event(int epoll_fd, data_t* d);

int run_event_loop(event_loop_t* event_loop);


#endif