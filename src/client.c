#include <stdint.h>

#define DISCONNECTED 1<<0
#define CONNECTECTED 1<<1
#define WAIT_FOR_REPLY 1<<2 // client is waiting for a reply from server
#define CLOSED 1<<3

typedef struct client_ds{
    int conn_fd; // the socket descriptor assosiciated with the client
    int file_fd; // the file associated with the client. TO-DO : make it shared 
    uint8_t client_state;
} client_t;


