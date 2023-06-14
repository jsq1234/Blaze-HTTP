#include "../include/utils.h"
#include <arpa/inet.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/fcntl.h>
#include <sys/socket.h>
#include <unistd.h>
#include <stdint.h>

int read_file(FILE *fptr, long f_size, unsigned char* buffer){
    size_t bytes = 0;
    unsigned char* ptr = buffer;
    
    while(1){
        bytes = fread(ptr,sizeof(char),RD_BUF,fptr);
        ptr[bytes] = '\0';
        if( bytes != RD_BUF ) {
            break;
        }
        ptr += bytes;
    }

    return 0;
}

int read_large_file(FILE* fptr, long f_size, unsigned char* buffer){
    size_t bytes = 0;
    unsigned char* ptr = buffer;

    while(1){
        bytes = fread(ptr,sizeof(unsigned char),BIG_BUF,fptr);
        if( bytes != BIG_BUF ){
            break;
        }
        ptr += bytes;
    }
    return 0;
}

const char* get_file_from_url(char *url, size_t url_len){
    char* ptr = url + url_len - 1;
    while( *ptr != '.' ) --ptr;
    return ptr + 1;
}

int get_content_type(char* buf, const char* url){
    int url_size = strlen(url);

    url += url_size - 1; // get to the last index
    
    while( *url != '.' ) --url;
    
    ++url;
 
    if( strcmp(url,"html") == 0 ){

        strcpy(buf,"text/html");

    }else if( strcmp(url,"css") == 0 ){
        
        strcpy(buf,"text/css");

    }else if( strcmp(url, "js") == 0 ){
        
        strcpy(buf,"text/javascript");

    }else if( strcmp(url,"png") == 0 ){
        
        strcpy(buf,"image/png");

    }else if( strcmp(url,"jpg") == 0 ){
        
        strcpy(buf,"image/jgp");

    }else if( strcmp(url,"jpeg") == 0 ){
        
        strcpy(buf, "image/jpeg");

    }else if( strcmp(url, "avif") == 0 ){
        
        strcpy(buf, "image/avif");
    }

    return 0;
}
















