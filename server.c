#include <netdb.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h> // read(), write(), close()
#define MAX 80
#define PORT 8000
#define SA struct sockaddr

// Function designed for chat between client and server.
char **read_message(int connfd);
void send_message(int connfd, char *reply);
char *get_file(char *);
// Driver function
int main() {
  int sockfd, connfd, len;
  struct sockaddr_in servaddr, cli;

  // socket create and verification
  sockfd = socket(AF_INET, SOCK_STREAM, 0);
  if (sockfd == -1) {
    printf("socket creation failed...\n");
    exit(0);
  } else
    printf("Socket successfully created..\n");
  bzero(&servaddr, sizeof(servaddr));

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
  len = sizeof(cli);

  // Accept the data packet from client and verification
  connfd = accept(sockfd, (SA *)&cli, &len);
  if (connfd < 0) {
    printf("server accept failed...\n");
    exit(0);
  } else
    printf("server accept the client...\n");

  // Function for chatting between client and server
  //
  char **message1 = read_message(connfd);

  printf("Client request!\n");

  char reply1[2048];

  memset(reply1, 0, sizeof(reply1));

  char *html = get_file("index.html");

  snprintf(reply1, sizeof(reply1),
           "HTTP/1.1 200 OK\nDate: Blah\nContent-Length: %ld\nContent-Type: "
           "text/html\n\r\n",
           strlen(html));

  strcat(reply1, html);

  send_message(connfd, reply1);

  char **message2 = read_message(connfd);

  char reply2[2048];

  char *css = get_file("styles.css");

  snprintf(reply2, sizeof(reply2),
           "HTTP/1.1 200 OK\nDate: Blah\nContent-Length: %ld\nContent-Type: "
           "text/css\n\r\n",
           strlen(css));

  strcat(reply2, css);

  send_message(connfd, reply2);
  // After chatting close the socket
  close(sockfd);
}

char *get_file(char *filepath) {
  FILE *fptr = fopen(filepath, "r");
  char buffer[150];
  memset(buffer, 0, sizeof(buffer));
  char *reply = (char *)malloc(1000);
  memset(reply, 0, 1000);

  while (fgets(buffer, 150, fptr) != NULL) {
    strcat(reply, buffer);
  }

  return reply;
}
void send_message(int connfd, char *reply) {
  int len = strlen(reply);
  int left = len;
  while (left != 0) {
    int bytesSend = send(connfd, reply, left, 0);
    left -= bytesSend;
    reply += bytesSend;
  }
}
char **read_message(int connfd) {
  char **lines = (char **)malloc(100 * sizeof(char *));
  char buffer[1024];
  memset(buffer, 0, sizeof(buffer));

  int i = 0;
  while (1) {

    ssize_t bytesRcvd = recv(connfd, buffer, sizeof(buffer), 0);
    buffer[bytesRcvd] = '\0';

    if (bytesRcvd == 0) {
      printf("Connection closed by the client.\n");
      exit(1);
    }
    lines[i] = (char *)malloc(strlen(buffer) + 1);
    strcpy(lines[i], buffer);

    int len = strlen(buffer);
    i++;

    if (buffer[len - 1] == '\n' && buffer[len - 2] == '\r') {
      break;
    }
  }

  lines[i] = "\0";

  return lines;
}
