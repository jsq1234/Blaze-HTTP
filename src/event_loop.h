#ifndef EVENT_LOOP_HEADER
#define EVENT_LOOP_HEADER

#include "client.h"
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/epoll.h>
#include <sys/socket.h>

#define MAX_CONN 4 * 1024
#define MAX_EVENT 8 * 1024
#define BZ_NONE 1 << 0
#define BZ_READABLE 1 << 1
#define BZ_WRITEABLE 1 << 2
#define BZ_EDGE_TRIG 1 << 3
#define BZ_ALL (BZ_READABLE | BZ_WRITEABLE | BZ_NONE | BZ_EDGE_TRIG)

struct event_loop_ds;

typedef struct event_loop_ds event_loop_t;

typedef struct event_handler {
	void (*bz_handle_new_connection)(event_loop_t *event_loop, data_t *d);
	void (*bz_handle_read_event)(event_loop_t *event_loop, data_t *d);
	void (*bz_handle_write_event)(event_loop_t *event_loop, data_t *d);
	void (*bz_handle_close_event)(event_loop_t *event_loop, data_t *d);
} bz_event_t;

typedef void bz_event_handler_t(int e, data_t *d);

typedef struct epoll_struct {
	int epollfd;
	struct epoll_event events[MAX_EVENT];
	int max_size;
} bz_epoll_t;

typedef struct count {
	size_t read;
	size_t written;
	size_t connected;
	size_t disconnected;
	size_t err;
} count_t;

struct event_loop_ds {
	bz_epoll_t *epoll_data;
	int maxfd;
	int flags;
	count_t usage;
	bz_connection_t *connections[MAX_CONN];
	bz_event_handler_t *event_handler;
	bz_event_t handler;
};

event_loop_t *bz_create_event_loop(size_t size);

int bz_add_event(int epfd, data_t *d, int flags);
int bz_delete_event(int epfd, data_t *d);

void bz_handle_new_connection(event_loop_t *event_loop, data_t *d);
void bz_read_event(event_loop_t *event_loop, data_t *d);
void bz_write_event(event_loop_t *event_loop, data_t *d);
void bz_close_event(event_loop_t *event_loop, data_t *d);

int bz_run_event_loop(event_loop_t *event_loop);
int bz_process_events(event_loop_t *event_loop);

#endif