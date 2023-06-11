#ifndef HTTP_PARSER
#define HTTP_PARSER

#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <string.h>

#define MOVE(p,l,n) p += n; l -= n;
#define TRAVERSE_TILL(p,ch) while( *p != '\0' && *p != ch ) ++p; 
#define CHECK_EOF(l,p) if( l == 4 && strncmp(p,"\r\n\r\n", 4) == 0 ){ return 0; }

extern const char* not_found_reply;
extern const char* not_implemented_reply;

typedef struct http_headers {
  char connection[10];
  char content_type[30];
  char content_length[8];
} http_header_t;

typedef struct http_request {
  char method[10];
  char url[200];
  char version[10];
  http_header_t headers;
} http_t;

extern http_t request;

int parse_request(const char *msg,size_t len, http_t *request);

#endif
