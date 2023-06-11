#ifndef UTILS_HEADER
#define UTILS_HEADER

#include <stdint.h>
#include <stdio.h>
#include <string.h>
#define RD_BUF 512 // the amount of bytes fread() reads in one call 
int read_text_file(FILE* fptr, long f_size, char* buf);
const char* get_file_from_url(char* url, size_t url_size);
int get_content_type(char* buf, const char* url, size_t url_size);
#endif
