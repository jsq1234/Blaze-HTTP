#include "client.h"
#include "event_loop.h"
#include "networking.h"
#include <netinet/in.h>

typedef struct server_st{
    struct sockaddr_in sadr;
    socklen_t len;
    event_loop_t* event_loop;
    data_t* d;
} server_t;

void init_server(server_t* Server, uint16_t port);
int main(int argc, char** argv){
    if( argc < 2 ){
        fprintf(stderr, "Usage : %s [port]\n", argv[0]);
        exit(1);
    }

    uint16_t port = atoi(argv[1]);

    data_t d = {
        .buff = NULL,
        .buff_size = 0,
        .f_size = 0,
        .fd = 0,
        .offset = 0,
        .state = 0,
        .filefd = -1,
    };

    event_loop_t* event_loop = bz_create_event_loop(100000);
    
    server_t Server;
    Server.d = &d;
    Server.event_loop = event_loop;
    
    init_server(&Server,port);
    bz_run_event_loop(Server.event_loop);
}

void init_server(server_t* Server, uint16_t port){
    
    int fd = bz_create_socket("tcp");

    memset(&Server->sadr,0,sizeof(struct sockaddr_in));
    memset(&Server->len,0,sizeof(socklen_t));

    Server->sadr.sin_family = AF_INET;
    Server->sadr.sin_port = htons(port);
    Server->sadr.sin_addr.s_addr = INADDR_ANY;

    if( bz_set_socket_nonblocking(fd) < 0 ){
        fprintf(stderr,"Cannot make socket non-blocking. Exiting...\n");
        exit(1);
    }

    if( bz_set_so_reuse_addr(fd) < 0 ){
        fprintf(stderr, "SO_REUSEADDR failed.\n");
        exit(1);
    }

    if( bz_set_so_reuse_port(fd) < 0 ){
        fprintf(stderr, "SO_REUSEPORT failed.\n");
        exit(1);
    }

    /* Support for TCP_NODELAY and others will be added later. */

    bz_start_listening(fd, 10000);
}
