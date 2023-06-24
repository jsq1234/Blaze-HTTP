#ifndef EVENT_LOOP_HEADER
#define EVENT_LOOP_HEADER

#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/epoll.h>
#include <sys/socket.h>

#define BZ_NONE 1<<0
#define BZ_READABLE 1<<1
#define BZ_WRITEABLE 1<<2
#define BZ_EDGE_TRIG 1<<3
#define BZ_ALL (BZ_READABLE|BZ_WRITEABLE|BZ_NONE|BZ_EDGE_TRIG)


typedef struct data{
    int fd; 
    int filefd; /* Negative for server sockfd */
    off_t offset; /* File offset/ Bytes sent */
    off_t f_size; /* File size */
    u_char* buff; /* A buffer to hold data */
    int state; /* State of the client. Use it only for the client. */
} data_t;

struct event_loop_ds;

typedef struct event_loop_ds event_loop_t;

typedef void bz_event_handler_t(data_t* d);

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
void bz_add_event(event_loop_t* event_loop, int fd,int flags);
void bz_delete_event(event_loop_t* event_loop, int fd, int del_mask);

void bz_handle_new_connection(data_t* d);
void bz_handle_read_event(data_t* d);
void bz_handle_write_event(data_t* d);
void bz_handle_close_event(data_t* d);

int run_event_loop(event_loop_t* event_loop);


#endif