#include "client.h"
#include "event_loop.h"
#include <netinet/in.h>

typedef struct server_st{
    struct sockaddr_in sadr;
    socklen_t len;
    event_loop_t* event_loop;
    data_t* d;
} server_t;

int main(int argc, char** argv){
    if( argc < 2 ){
        fprintf(stderr, "Usage : %s [port]\n", argv[0]);
        exit(1);
    }

    uint16_t port = atoi(argv[1]);

    

}