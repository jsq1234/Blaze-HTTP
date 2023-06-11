#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <stdlib.h>
#include <sys/stat.h>
#include "../include/utils.h"
#include "../include/server.h"

server_t server;

int init_server(uint16_t port) {

    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        perror("socket()");
        exit(1);
    }
    memset(&server,0,sizeof(server));

    server.info.sin_family = AF_INET;
    server.info.sin_port = htons(port);
    server.info.sin_addr.s_addr = INADDR_ANY;

    if (bind(sockfd, (const struct sockaddr *)&server.info, sizeof(server.info)) <
            0) {
        perror("bind()");
        exit(1);
    }

    make_socket_nonblocking(sockfd);

    if (listen(sockfd, 5) < 0) {
        perror("listen()");
        exit(1);
    }
    return 0;
}

int make_socket_nonblocking(int fd) {
    int flags = fcntl(fd, F_GETFL, 0);
    if (flags < 0) {
        perror("fcntl()");
        exit(1);
    }
    flags |= O_NONBLOCK;

    if (fcntl(fd, F_SETFL, flags) < 0) {
        perror("fcntl()");
        exit(1);
    }
    return 0;
}

int send_response(http_t* request, int sockfd){
    if( strcmp(request->method, "GET") != 0 ){
        return send_all(sockfd,strlen(not_implemented_reply),not_implemented_reply);
    }else{

        char* file_path = "";
        
        if( strcmp(request->url, "/") == 0 ){
            file_path = "index.html";
        }else{
            file_path = &request->url[1];
        }

        FILE* fptr = fopen(file_path, "r");
        if( fptr == NULL ){
            // handle file not found error
            // return -1 in case the local send() socket buffer is full
            return send_all(sockfd,strlen(not_found_reply),not_found_reply);
        }
        
        size_t reply_len = OK_reply(fptr,file_path,request);
        if( reply_len != -1 )
            return send_all(sockfd,reply_len,server.reply);

        return -1; 
        
    }
}

int send_all(int sockfd, size_t len, const char* reply){
    ssize_t bytes = 0;
    while(len){
        bytes = send(sockfd,reply,len,0);
        if( bytes <= 0 ){
            if( errno == EAGAIN || errno == EWOULDBLOCK ){
                // the send() buffer is full, retry it later
                return -1;
            }
        }
        reply += bytes;
        len -= bytes;
    }
    return 0;
}

size_t OK_reply(FILE* fptr, const char* file_path, http_t* request){
    
    struct stat file_info;
    if( stat(file_path,&file_info) < 0 ){
        perror("stat()");
        return -1;
    }

    long f_size = file_info.st_size;

    char response_header[512]; 
    response_header[0] = '\0';

    const char* status_code = "200";
    const char* status_msg = "OK";
    char content_type[50] = "";

    get_content_type(content_type, request->url, strlen(request->url));

    snprintf(response_header,512, 
            "%s %s %s\r\n"
            "Content-Type: %s\r\n"
            "Conntection: keep-alive\r\n"
            "Content-Length: %ld\r\n\r\n",
            request->version, status_code, status_msg,
            content_type,
            f_size);

    // +1 for null terminating character
    size_t reply_size = strlen(response_header) + f_size + 1;

    //DO NOT FORGET TO FREE AFTER SENDING THE REPLY TO THE CLIENT!
    server.reply = (char*)malloc(sizeof(char)*reply_size);

    char* ptr = server.reply + strlen(response_header); 

    read_text_file(fptr,f_size,ptr);

    return reply_size;
}

