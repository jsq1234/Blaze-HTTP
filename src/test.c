#define _GNU_SOURCE
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

#define READ (1<<1)
#define WRITE (1<<2)
#define CLOSE (1<<3)
#define ALL (READ|WRITE|CLOSE)

#include <sys/socket.h>
#include <netinet/in.h>
int main(){
    
    #ifdef HAVE_ACCEPT4
        printf("have accept4\n");
    #else
        printf("doesn't have accept4\n");
    #endif
}