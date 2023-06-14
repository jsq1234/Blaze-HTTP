#ifndef HTTP_HEADER

#define HTTP_HEADER 


#define MOVE(p, l, n)                                                          \
  p += n;                                                                      \
  l -= n;
#define TRAVERSE_TILL(p, ch)                                                   \
  while (*p != '\0' && *p != ch)                                               \
    ++p;
#define CHECK_EOF(l, p)                                                        \
  if (l == 4 && strncmp(p, "\r\n\r\n", 4) == 0) {                              \
    return 0;                                                                  \
  }


#include <stdio.h>
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

enum {
  NOT_FOUND = -8,
  NOT_IMPLEMENTED,
};

extern const char *not_found_reply; 
extern const char *not_implemented_reply;

int parse_request(const char *msg, size_t len, http_t *request);

#endif