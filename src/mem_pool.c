#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>

#define BZ_DEFAULT_POOL_SIZE                                                   \
  16 * 1024 // keep 16 KB as the default memory pool size

#define BZ_ALIGNMENT 4 * 1024 // for paged aligned memory

// The data structure defines the pool which will be allocated
// This is a generic pool which divides the created pool into
// fixed-sized slabs/region. Suitable for allocating data
// structures which are all of the same type

struct bz_pool_ds {
  struct bz_pool_ds *next; // pointer to the next pool
  u_char *end;             // pointer to the end of the pool
  u_char *last;            // pointer to the last allocated element
  size_t size;             // total size of the pool
  size_t reg_size;         // size of region
};

typedef struct bz_pool_ds bz_pool_t;

bz_pool_t *bz_create_pool(size_t size, size_t region_size) {
  bz_pool_t *pool;
  bz_pool_t *prev = NULL;

  size_t total = 0;
  size_t meta_data = sizeof(bz_pool_t);

  while (size) {

    size_t alloc_size =
        size > BZ_DEFAULT_POOL_SIZE ? BZ_DEFAULT_POOL_SIZE : size;

    size -= alloc_size;
    total += alloc_size;

    alloc_size += meta_data;

    if (posix_memalign((void **)&pool, BZ_ALIGNMENT, alloc_size) < 0) {
      return NULL;
    }
    pool->end = (u_char *)pool + alloc_size;
    pool->last = (u_char *)pool + sizeof(bz_pool_t);
    pool->size = alloc_size - sizeof(bz_pool_t);
    pool->reg_size = region_size;
    pool->next = NULL;

    u_char *ptr = pool->last;
    u_char *end = pool->end;

    if (prev) {
      prev->next = pool;
    }
    prev = pool;
  }
  printf("pool->reg_size : %zu\n", pool->reg_size);
  printf("done pool creation...\n");
  return pool;
}

void *bz_palloc(bz_pool_t *pool, size_t size) {
  assert(
      size == pool->reg_size &&
      "given data structure size is not equal to the block size of the pool");

  bz_pool_t *cur = pool;
  u_char *ptr = cur->last;

  while (cur) {

    if ((size_t)(cur->end - ptr) >= cur->reg_size) {
      cur->last += cur->reg_size;
      cur->size = cur->end - cur->last;
      return ptr;
    }
    cur = cur->next;
  }

  return NULL;
}

static inline bz_pool_t* bz_create_pool_default(size_t region_size) {
  return bz_create_pool(BZ_DEFAULT_POOL_SIZE, region_size);
}

typedef struct point {
  int x;
  int y;
  int z;
  char pad[6];
} point_t;

int main() {
  bz_pool_t* pool = bz_create_pool_default(sizeof(point_t));
  printf("pool size : %ld, %ld\n", pool->end - pool->last, pool->size );

  point_t* p[10];
  for(int i=0; i<10; i++){
    p[i] = bz_palloc(pool,sizeof(point_t));
    p[i]->x = 12;
    p[i]->y = 13;
    p[i]->z = 12;
  }
  printf("pool size : %ld, %ld\n", pool->end - pool->last, pool->size );


}