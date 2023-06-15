#ifndef UTILS_HEADER
#define UTILS_HEADER

#include <stdint.h>
#include <stdio.h>
#include <string.h>


// Color Codes -> 

#define RED "\x1b[31m"
#define GREEN "\x1b[32m"
#define YELLOW "\x1b[33m"
#define BLUE "\x1b[34m"
#define MAGENTA "\x1b[35m"
#define CYAN "\x1b[36m"
#define RESET "\x1b[0m"



#define RD_BUF 4*1024 // 4KB for small files.
#define BIG_BUF 16*1024 // 16 KB buffer for reading large file
#define LARGE_FILE 1024*1024 // Treat 1 MB file as large for now, and use BIG_BUF in fread()

// use for text files that are small ( < LARGE_FILE )
size_t read_file(FILE* fptr, size_t f_size, unsigned char* buf);
// use for large files
//
size_t read_large_file(FILE* fptr, size_t f_size, unsigned char* buf);

const char* get_file_from_url(char* url, size_t url_size);

int get_content_type(char* buf, const char* url);

#endif
