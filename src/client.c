#include <stdint.h>
#include <stdio.h>
#include <sys/types.h>

#define DISCONNECTED 1<<0
#define CONNECTECTED 1<<1
#define WAIT_FOR_REPLY 1<<2 // client is waiting for a reply from server
#define CLOSED 1<<3

// typedef struct data{
//     int fd; 
//     int filefd; /* Negative for server sockfd */
//     off_t offset; /* File offset/ Bytes sent */
//     off_t f_size; /* File size */
//     u_char* buff; /* A buffer to hold data */
//     int state; /* State of the client. Use it only for the client. */
// } data_t;



