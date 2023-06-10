#include "../include/http.h"
#include "../include/utils.h"
#include <stdio.h>
#include <string.h>

int main(int argc, char *argv[]) {
  const char *msg = "GET /folder/sub_folder/poggers.js HTTP/1.1\r\n\r\n";
  memset(&request, 0, sizeof(request));
  parse_request(msg,strlen(msg),&request);
  printf("method: %s\n", request.method);
  printf("url: %s\n", request.url);
  printf("version: %s\n", request.version);
}
