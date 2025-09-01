#ifndef FILE_CACHE
#define FILE_CACHE
#include "./include/bufferpool.h"
#include "./include/lrucache.h"
#include <stddef.h>
#include <time.h>  


typedef struct cached_file {
    char filename[256];
    buffer_t *buffer;
    size_t file_size;
    time_t last_accessed;
    struct cached_file *next;
}cached_file_t;

typedef struct file_cache {
    buffer_pool_t *buffer_pool;
    cached_file_t *files[CACHE_SIZE];
    int cached_files_count;
}file_cache_t;

file_cache_t* create_file_cache();
char* read_file_cached(file_cache_t *cache, const char *filename);
file_cache_t* remove_cache_file(file_cache_t *cache, const char *filename);
file_cache_t* remove_cache_file(file_cache_t *cache, const char *filename);
file_cache_t* remove_mru_cached_file(file_cache_t *cache);

#endif