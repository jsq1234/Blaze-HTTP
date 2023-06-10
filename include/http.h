#ifndef HTTP_PARSER
#define HTTP_PARSER

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
int parse_request(const char *msg, http_t *request);
int generate_response(http_t *request, char *response);

#endif
