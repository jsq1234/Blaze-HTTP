#include <stdlib.h>
#include <string.h>
#include <sys/types.h>

#define BZ_DEFAULT_POOL_SIZE 4*1024
#define BZ_ALIGNMENT 16
// The data structure defines the pool which will be allocated
// This is a generic pool which divides the created pool into 
// fixed-sized slabs/region. Suitable for allocating data
// structures which are all of the same type

struct bz_pool_ds{
    struct bz_pool_ds* next; // pointer to the next pool
    void* end; // pointer to the end of the pool
    void* last; // pointer to the last allocated element
    size_t size; // total size of the pool 
    size_t reg_size; // size of region 
};

typedef struct bz_pool_ds bz_pool_t;

int bz_create_pool(bz_pool_t* pool, size_t size, size_t region_size){
    bz_pool_t* cur = pool;
    bz_pool_t* prev = NULL;

    size_t total = 0;

    while(size){
        
        size_t alloc_size = size > BZ_DEFAULT_POOL_SIZE ? BZ_DEFAULT_POOL_SIZE : size;
        if( posix_memalign((void**)&cur,BZ_ALIGNMENT,alloc_size)  < 0 ){
            return -1;
        }
        cur->end = cur + alloc_size;
        cur->last = cur + sizeof(bz_pool_t);
        cur->size = alloc_size - sizeof(bz_pool_t);
        cur->reg_size = region_size;
        cur->next = NULL;

        if( prev ){
            prev->next = cur;
            prev = cur;
        }

        size -= alloc_size;
    }

    return 0;
}

static inline
int bz_create_pool_default(bz_pool_t* pool, size_t region_size){
    return bz_create_pool(pool,BZ_DEFAULT_POOL_SIZE,region_size);
}