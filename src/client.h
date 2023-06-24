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
#define PENDING_REPLY 1<<4
#define CLOSED 1<<5

struct client_connection_ds;
typedef client_connection_ds client_connection_t;

typedef struct data{
    int fd; 
    int filefd; /* Negative for server sockfd */
    off_t offset; /* File offset/ Bytes sent */
    off_t f_size; /* File size */
    u_char* buff; /* A buffer to hold data */
    int state; /* State of the client. Use it only for the client. */
} data_t;

#endif