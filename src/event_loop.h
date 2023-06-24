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
    void (*accept_connection)(event_loop_t* event_loop, int fd);
    void (*delete_connection)(event_loop_t* event_loop, int fd);
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

/* To be put later in another header file  */

/*  The difference between disconnected and closed it that 
    in case client instantly shutdown his connection (SHUT_WR),
    also known as graceful disconnect, and if we have to send reply
    we will send the reply and put the client in the closed state.
    When client is in closed state, close(fd) will be called.
 */

#define CONNECTED 1<<1
#define DISCONNECTED 1<<2
#define WAITINT_REPLY 1<<3
#define CLOSED 1<<4

struct client_connection_ds;
typedef client_connection_ds client_connection_t;

typedef uint16_t client_state_t;
/* For future use */
typedef struct{
    int filefd;
} client_data_t;

/*  These handlers are used to handle read and write events.
    The specific function to use is determined at the time of event
 */

typedef void (*read_handler)(client_connection_t* c, u_char* buffer);
typedef void (*write_handler)(client_connection_t* c);

/*  
    This data structure defines client data associated with a client connection
    This function is what will be returned in case of an event in the 
    event.data.ptr field of epoll_event data structure. 
*/

struct client_connection_ds {
    int fd;
    int filefd;
    read_handler r_client;
    write_handler wr_client;
    client_state_t client_state;
};

event_loop_t* bz_create_event_loop(size_t size);
void bz_add_event(event_loop_t* event_loop, int op, int flags);
void bz_delete_event(event_loop_t* event_loop, int op, int del_mask);
int run_event_loop(event_loop_t* event_loop);


#endif