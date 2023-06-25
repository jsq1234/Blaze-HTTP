#ifndef BZ_BUFFER_HEADER
#define BZ_BUFFER_HEADER

#include <stddef.h>

typedef struct buffer{
    void* buff;
    size_t len;
} bz_buffer_t;

#endif