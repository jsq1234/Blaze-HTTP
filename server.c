#include <asm-generic/errno-base.h>
#include <netdb.h>
#include <netinet/in.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <error.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h> // read(), write(), close()

#define MAX 80
#define PORT 8000
#define SA struct sockaddr
#define BUF_SIZE 1024
#define FILE_NOT_FOUND ENOENT

typedef struct _http_request_headers {
  char *host;
  char connection[15];
  char content_type[100];
  unsigned int content_length;
  char *accept[20];
  char *accept_language;
  char *accept_encoding;
} HttpRequestHeaders;

typedef struct _http_response_headers {
  const char *content_type;
  const char *content_length;
  const char *content_encoding;
  const char *connection;
} HttpResponseHeaders;

struct HttpRequest {
  char method[10]; // GET, POST, DELETE
  char *URL;
  char version[10];
  HttpRequestHeaders headers;
  char *body;
};
struct HttpResponse {
  const char *protocol_version[20];
  const char *status_code[4];
  const char *status_text[100];
  HttpResponseHeaders headers;
};

char buffer[BUF_SIZE];
const char *acceptable_headers[] = {
    "Host",   "Connection",      "Content-Type",   "Content-Length",
    "Accept", "Accept-Language", "Accept-Encoding"};

int concat(char *restrict, const char *restrict, size_t, size_t);
void parse(struct HttpRequest *request, char *message);
int get_str(char *ptr, const char delim);
void update_fields(const char *m, HttpRequestHeaders *header, char *ptr);
void read_request(struct HttpRequest *request, int connfd);
void handle_request(struct HttpRequest *request, int connfd);
void send_all(int connfd, char *msg);
void handle_error(int err, int connfd);
char *read_file(char *filepath, FILE *fptr);
// Driver function
int main() {
  int sockfd, connfd;
  struct sockaddr_in servaddr, cli;
  socklen_t len = sizeof(cli);

  // socket create and verification
  sockfd = socket(AF_INET, SOCK_STREAM, 0);
  if (sockfd == -1) {
    printf("socket creation failed...\n");
    exit(0);
  } else
    printf("Socket successfully created..\n");
  if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &(int){1}, sizeof(int)) < 0)
    perror("setsockopt(SO_REUSEADDR) failed");
  memset(&servaddr, 0, sizeof(servaddr));
  memset(&cli, 0, sizeof(cli));
  memset(buffer, 0, sizeof(buffer));

  // assign IP, PORT
  servaddr.sin_family = AF_INET;
  servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
  servaddr.sin_port = htons(PORT);

  // Binding newly created socket to given IP and verification
  if ((bind(sockfd, (SA *)&servaddr, sizeof(servaddr))) != 0) {
    printf("socket bind failed...\n");
    exit(0);
  } else
    printf("Socket successfully binded..\n");

  // Now server is ready to listen and verification
  if ((listen(sockfd, 5)) != 0) {
    printf("Listen failed...\n");
    exit(0);
  } else
    printf("Server listening..\n");

  // Accept the data packet from client and verification
  connfd = accept(sockfd, (SA *)&cli, &len);
  if (connfd < 0) {
    printf("server accept failed...\n");
    exit(0);
  } else
    printf("server accept the client...\n");

  while (1) {
    struct HttpRequest request;

    memset(&request, 0, sizeof(request));
    read_request(&request, connfd);
    // printf("poggers!\n");
    handle_request(&request, connfd);
   
    
  }
}

int concat(char *restrict dst, const char *restrict src, size_t n,
           size_t size) {
  if (sizeof(dst) + sizeof(src) + sizeof(char) > size) {
    return -1;
  }
  strncat(dst, src, n);
  return 0;
}

void parse(struct HttpRequest *request, char *message) {
  int i = 0;
  char *ptr1 = message;
  int len = 0;

  len = get_str(ptr1, ' ');
  strncpy(request->method, ptr1, len);

  ptr1 += len + 1;

  len = get_str(ptr1, ' ');
  request->URL = strndup(ptr1, len);

  ptr1 += len + 1;

  len = get_str(ptr1, '\r');
  strncpy(request->version, ptr1, len);

  ptr1 += len + 2;

  len = get_str(ptr1, ':');
  ptr1 += len + 2;
  len = get_str(ptr1, '\r');
  request->headers.host = strndup(ptr1, len);

  ptr1 += len + 2;

  while (strncmp(ptr1, "\r\n", 2) != 0) {
    len = get_str(ptr1, ':');
    int present = 0;
    for (int i = 0; i < 7; i++) {
      if (strncmp(ptr1, acceptable_headers[i], len) == 0) {
        present = 1;
        ptr1 += len + 2;
        update_fields(acceptable_headers[i], &request->headers, ptr1);
        break;
      }
    }
    if (!present) {
      // printf("Cannot accept the current header.\n");
      len = get_str(ptr1, '\r');
      ptr1 += len + 2;
    }
  }
}

void update_fields(const char *m, HttpRequestHeaders *header, char *ptr1) {
  int len = 0;
  len = get_str(ptr1, '\r');
  if (strcmp(m, "Connection") == 0) {
    strncpy(header->connection, ptr1, len);
  } else if (strcmp(m, "Content-Type") == 0) {
    strncpy(header->content_type, ptr1, len);
  } else if (strcmp(m, "Content-Length") == 0) {
    char cpy[10];
    memset(cpy, 0, sizeof(cpy));
    strncpy(cpy, ptr1, len);
    header->content_length = atoi(cpy);
  } else if (strcmp(m, "Accept") == 0) {
    char *val = strndup(ptr1, len);
    char *token;
    char *delim = ",";

    token = strtok(val, delim);
    char **ptr2 = header->accept;
    while (token != NULL) {
      *ptr2 = strdup(token);
      ptr2++;

      token = strtok(NULL, delim);
    }
    *ptr2 = "\0";
  } else if (strcmp(m, "Accept-Language") == 0) {
    header->accept_language = strndup(ptr1, len);
  } else if (strcmp(m, "Accept-Encoding") == 0) {
    header->accept_encoding = strndup(ptr1, len);
  }
  ptr1 += len + 2;
}

int get_str(char *ptr, const char delim) {
  char *itr = ptr;
  while (*itr != delim)
    itr++;
  int len = itr - ptr;
  return len;
}

void read_request(struct HttpRequest *request, int connfd) {
  // assuming the http fileds are just 5000 bytes
  // to-do change it to something better
  char message[800];

  memset(message, 0, sizeof(message));
  int total_length = 0;
  int loop = 0;
  while (1) {
    // read from connfd and store it in buffer. It is a blocking system call.
    // to-do make it non-blocking
    ssize_t bytesRcvd = recv(connfd, buffer, BUF_SIZE, 0);
    if (bytesRcvd <= 0) {
      if (bytesRcvd == 0) {
        // bytesRcvd == 0 indicates that the client has gracefully closed the
        // connection
        printf("Client Closed Conenction\n");
        return;
      }
      // for now, exit due to error.
      // to-do -> change it later
      perror("recv() error");
    }

    buffer[bytesRcvd] = '\0';

    // concat it to the end of message.
    // if you are concating more than the size of message, exit with in error
    // can be done in a better way.
    if (concat(message, buffer, strlen(buffer), 6000) < 0) {
      fprintf(stderr, "buffer overflow!\n");
      exit(1);
    }
    // printf("%s",buffer);
    int len = strlen(buffer);
    total_length += len;

    // if the last line contains \n\r, we have recieved all the request message
    // at least in the cause of HTTP GET request
    //  last 4 bytes are \r\n\r\n then means the request is complete;
    if (strncmp(buffer + len - 4, "\r\n\r\n", 4) == 0) {
      //printf("%s", message);
      // printf("total length: %d\n",total_length);
      break;
    }
  }

  parse(request, message);
}

void handle_request(struct HttpRequest *request, int connfd) {

  if (strcmp(request->method, "GET") == 0) {
    if (strcmp(request->URL, "/") == 0) {
      char *filepath = "index.html";
      FILE *fptr = fopen(filepath, "r");
      if (fptr == NULL) {
        handle_error(errno, connfd);
        errno = 0;
        return;
      }

      char *page = read_file(filepath, fptr);
      int content_length = strlen(page);

      char *response_message =
          (char *)calloc(content_length + 300, sizeof(char));

      snprintf(response_message, content_length + 300,
               "HTTP/1.1 200 OK\r\nContent-Length: %d\r\nContent-Type: "
               "text/html\r\n\r\n",
               content_length);

      int len = strlen(response_message);
      strcat(response_message, page);

      send_all(connfd, response_message);
      // printf("%s",response_message);
      // printf("send!\n");
    } else {
      char *filepath = ++request->URL;
      FILE *fptr = fopen(filepath, "r");
      if (fptr == NULL) {
        handle_error(FILE_NOT_FOUND, connfd);
        return;
      }
      char *page = read_file(filepath, fptr);
      int content_length = strlen(page);
      char *response_message =
          (char *)calloc(content_length + 300, sizeof(char));
      snprintf(response_message, content_length + 300,
               "HTTP/1.1 200 OK\r\nContent-Length: %d\r\nContent-Type: "
               "text/css\r\n\r\n",
               content_length);

      int len = strlen(response_message);
      strcat(response_message, page);

      send_all(connfd, response_message);
    }
  }
}


void send_all(int connfd, char *msg) {
  int left = strlen(msg);
  while (left) {
    ssize_t bytesSnd = send(connfd, msg, left, 0);
    if (bytesSnd <= 0) {
      perror("send()");
    }
    left -= bytesSnd;
    msg += bytesSnd;
  }
}

char *read_file(char *filepath, FILE *fptr) {
  int i = 0;
  char *page = (char *)malloc(sizeof(char) * 100);
  int size = 100;

  while (1) {
    int ch = fgetc(fptr);
    if (ch == EOF)
      break;
    if (i < size) {
      page[i] = ch;
    } else {
      // if our arr is full, reallocate it with size twice of its current size
      page = (char *)realloc(page, 2 * size);
      size *= 2;
      page[i] = ch; // assign the currently read character;
    }
    i++;
  }
  page[i] = '\0';
  return page;
}

void handle_error(int err, int connfd) {
  if( err == FILE_NOT_FOUND ){
    FILE* fptr = fopen("error_html/error_404.html","r");
    char* page = read_file("error_404.html",fptr);
    int content_length = strlen(page);

    char* error_reply = (char*)calloc(content_length + 300,sizeof(char));
    
    snprintf(error_reply,content_length+300,
    "HTTP/1.1 404 Not Found\r\nContent-Length: %d\r\nContent-Type: text/html\r\n\r\n",
    content_length);
    strcat(error_reply,page);

    send_all(connfd,error_reply);
    //printf("sent message: %s",error_reply);

  }
}