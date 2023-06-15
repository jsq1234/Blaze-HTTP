#include "../include/utils.h"
#include <arpa/inet.h>
#include <fcntl.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/fcntl.h>
#include <sys/socket.h>
#include <unistd.h>
#include <stdint.h>

size_t read_file(FILE* fptr, size_t f_size, unsigned char* dest)
{
   size_t total_bytes = 0;
    while (f_size) {
        size_t chunk = f_size < BIG_BUF ? f_size : BIG_BUF;
        size_t n = fread(&dest[total_bytes], 1, chunk, fptr);
        if (!n)
            break;
        total_bytes += n;
        f_size -= n;
    }
    dest[total_bytes] = '\0';
    return total_bytes;
}
size_t read_large_file(FILE* fptr, size_t f_size, unsigned char* dest)
{
   size_t total_bytes = 0;
    while (f_size) {
        size_t chunk = f_size < BIG_BUF ? f_size : BIG_BUF;
        size_t n = fread(&dest[total_bytes], 1, chunk, fptr);
        if (!n)
            break;
        total_bytes += n;
        f_size -= n;
    }
    dest[total_bytes] = '\0';
    return total_bytes;
}
/*
size_t read_large_file(FILE* fptr, long f_size, unsigned char* buffer){
    size_t bytes = 0;
    unsigned char* ptr = buffer;
    size_t total_bytes = 0;
    while(1){
        bytes = fread(ptr,1,BIG_BUF,fptr);
        total_bytes += bytes;
        if( bytes != BIG_BUF ){
            break;
        }
        ptr += bytes;
    }
    return total_bytes;
}
 */ 
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
















