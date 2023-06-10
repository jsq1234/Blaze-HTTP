#include "../include/http.h"
#include "../include/utils.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

http_t request;
int parse_request(const char *msg, size_t msg_len, http_t *request) {
  const char *ptr = msg;
  size_t left = msg_len;
  
  if (strncmp(ptr, "GET", strlen("GET")) == 0) {
    strcpy(request->method, "GET");
    MOVE(ptr,left,4)
  } else {
    return -1;
  }
  const char *itr = ptr;
  TRAVERSE_TILL(itr, ' ')
  int len = itr - ptr;

  strncpy(request->url, ptr, len);
  request->url[len] = '\0';

  MOVE(ptr,left,len+1) // +1 for the space 
  itr = ptr;

  TRAVERSE_TILL(itr, '\r')
  len = itr - ptr;

  strncpy(request->version, ptr, len);
  request->version[len] = '\0';

  return 0;
  // to be implemented in the future
  // for now, only parses the first line, i.e the request line

  MOVE(ptr,left,len)
  // if we have reached EOF (\r\n\r\n) the return 
  CHECK_EOF(left,ptr)
  //move by 2 to go the next line 

  return 0;
}

int generate_response(http_t* request, char* response){
  const char* filepath = "";
  if( strcmp(request->url, "/") == 0 ){
    filepath = "index.html";
  }else{
    filepath = &request->url[1];
  }
  FILE* fptr = fopen(filepath,"r");
  if( fptr == NULL ){
    handle_error(request, response, FNOTFOUND);
  }
  
  
   
  return 0;
}
