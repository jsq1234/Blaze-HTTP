#ifndef UTILS_HEADER
#define UTILS_HEADER

#include <stdint.h>
#include <stdio.h>
#include <string.h>

#define RD_BUF 4*1024 // 4KB for small files.
#define BIG_BUF 16*1024 // 16 KB buffer for reading large file
#define LARGE_FILE 1024*1024 // Treat 1 MB file as large for now, and use BIG_BUF in fread()

// use for text files that are small ( < LARGE_FILE )
int read_file(FILE* fptr, long f_size, unsigned char* buf);
// use for large files.
int read_large_file(FILE* fptr, long f_size, unsigned char* buf);

const char* get_file_from_url(char* url, size_t url_size);

int get_content_type(char* buf, const char* url);

#endif
