#include "../include/http.h"
#include <stdlib.h>
#include <string.h>

http_t request;
int parse_request(const char *msg, http_t *request) {
  const char *ptr = msg;
  if (strncmp(ptr, "GET", strlen("GET")) == 0) {
    strcpy(request->method, "GET");
    ptr += 4;
  } else {
    return -1;
  }
  const char *itr = ptr;

  while (*itr != ' ')
    itr++;
  int len = itr - ptr;

  strncpy(request->url, ptr, len);

  request->url[len] = '\0';

  ptr = itr + 1;
  itr = ptr;

  while (*itr != '\r')
    itr++;

  len = itr - ptr;

  strncpy(request->version, ptr, len);

  request->version[len] = '\0';

  return 0;
}
