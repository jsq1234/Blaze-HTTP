#ifndef CLIENT_HEADER
#define CLIENT_HEADER

#include <netinet/in.h>
#include <stdint.h>

/*  The difference between disconnected and closed it that 
    in case client instantly shutdown his connection (SHUT_WR),
    also known as graceful disconnect, and if we have to send reply
    we will send the reply and put the client in the closed state.
    When client is in closed state, close(fd) will be called.
 */

#define CONNECTED 1<<1
#define DISCONNECTED 1<<2
#define REPLY_SENT 1<<3
#define CLOSED 1<<4

struct client_connection_ds;
typedef client_connection_ds client_connection_t;

typedef uint16_t client_state_t;

/* For future use */
typedef struct{
    int 			fd;
    int 			filefd;
    client_state_t	flags;
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
    client_data_t		data;
    struct sockaddr_in	sa;
    socklen_t			len;
    read_handler		r_client;
    write_handler		wr_client;
};

#endif