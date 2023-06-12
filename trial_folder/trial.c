#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

size_t read_bin_file(uint8_t *buffer, long f_size, FILE *fptr);
size_t write_bin_file(uint8_t *buffer, long f_size, FILE *fptr);

int main(int argc, char **argv) {
  if (argc < 2) {
    printf("Usage: %s [filepath]\n", argv[0]);
    exit(1);
  }
  char *file_name = argv[1];
  FILE *fptr = fopen(file_name, "rb");
  if (fptr == NULL) {
    perror("fopen()");
    exit(1);
  }

  struct stat info;
  if (stat(file_name, &info) < 0) {
    perror("stat()");
    exit(1);
  }
  long f_size = info.st_size;
  printf("file size: %ld\n", f_size);
  uint8_t *buffer = malloc(sizeof(uint8_t) * f_size);

  long l = read_bin_file(buffer, f_size, fptr);

  printf("file read : %ld\n", l);

  FILE *wptr = fopen("poggers.html", "wb");
  if (wptr == NULL) {
    perror("fopen()");
    exit(1);
  }
  long wl = write_bin_file(buffer, f_size, wptr);

  printf("file written size: %ld\n", wl);

  free(buffer);
  buffer = NULL;
}

size_t write_bin_file(uint8_t *buffer, long f_size, FILE *fptr) {
  uint8_t *ptr = buffer;
  const int SEND_SIZE = 10240;
  long total_bytes = 0;
  size_t bytes = fwrite(buffer, sizeof(uint8_t), f_size, fptr);

  return bytes;
}

size_t read_bin_file(uint8_t *buffer, long f_size, FILE *fptr) {
  uint8_t *ptr = buffer;
  size_t bytes = 0;
  const int RECV_SIZE = 1024;
  size_t total_bytes = 0;
  while (1) {
    bytes = fread(ptr, sizeof(uint8_t), RECV_SIZE, fptr);
    ptr += bytes;
    total_bytes += bytes;

    if (bytes != RECV_SIZE) {
      break;
    }
  }

  return total_bytes;
}
