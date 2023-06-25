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

/*  CONNECTED:
    Client is connected to the server and ready to send message
    Call read handler if the client is in connected state
    
    DISCONNECTED:
    Client has disconnected his side of connection, if there is
    any pending data left, send it to the client. 
    (To-Do) In case of keep-alive, implement a timer which will
    close the client socket after a while.
    
    REPLY_SENT:
    The all the reply was sent to the client, if read() returns 0,
    set the state to DISCONNECTED otherwise set the state to CONNECTED,
    in order to recieve more message.
    (To - Do ) In case of `Connection: closed`, change the state to DISCONNECTED/CLOSE
    
    PENDING_REPLY:
    The kernel buffer was full and therefore the whole reply could not be send.
    Call the write handler for such a client again.

    CLOSED:
    We have closed the client, remove it either using epoll_ctl_mod or 
    close(fd) system call.  
*/
#define CONNECTED 1<<1
#define DISCONNECTED 1<<2
#define MSG_RECVD 1<<3
#define REPLY_SENT 1<<4
#define PENDING_REPLY 1<<5
#define CLOSED 1<<6

struct connection_ds;

typedef struct connection_ds bz_connection_t;

typedef struct data{
    int     fd; 
    int     filefd;     /* Negative for server sockfd */
    off_t   offset;     /* File offset/ Bytes sent */
    off_t   f_size;     /* File size */
    u_char* buff;       /* A buffer to hold data */
    size_t buff_size;   /* Size of the buffer */
    int     state;      /* State of the client. Use it only for the client. */
} data_t;

struct connection_ds{
    struct sockaddr_in  sa;
    socklen_t           len;
    data_t              d;
};

#endif