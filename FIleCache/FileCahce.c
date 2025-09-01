#include <stdio.h>
#include <stdlib.h>
#include "./include/FileCache.h"
#include <fcntl.h>

file_cache_t* create_file_cache(){
    file_cache_t *cache = malloc(sizeof(file_cache_t));
    if(!cache) return NULL;
    cache->buffer_pool = create_buffer_pool(POOL_SIZE);
    if(!cache->buffer_pool ){
        free(cache);
        return NULL;
    }
    for(int i= 0; i<CACHE_SIZE;i++){
        cache->files[i] = NULL;
    }
    return cache;
}

char* read_file_cached(file_cache_t *cache, const char *filename)
{
    unsigned int index = hash(filename);

    cached_file_t *cached = cache->files[index];

    while(cached)
    {
        if(strcmp(cached->filename, filename) == 0)
        {
            cached->last_accessed = time(NULL);
            printf("File cache HIT for %s \n", filename);
            return cached->buffer->data;

        }

        cached = cached->next;
    }

    printf("File cache MISS for %s \n", filename);

    buffer_t *buffer = acquire_buffer(cache->buffer_pool);
    if(!buffer)
    {
        printf("No buffers are available");
        return NULL;
    }

    int fd = open(filename, O_RDONLY);
    if(fd == -1)
    {
        release_buffer(cache->buffer_pool, buffer);
        return NULL;
    }

    size_t bytes_read = read(fd, buffer->data, BUFFER_SIZE - 1);
    close(fd);

    if(bytes_read == -1)
    {
        release_buffer(cache->buffer_pool, buffer);
        return NULL;
    }

    buffer->data[bytes_read] = '\0';


//To add file as a new cache entry
cached_file_t *new_cached = malloc(sizeof(cached_file_t));

if(!new_cached)
{
    release_buffer(cache->buffer_pool, buffer);
    return NULL;
}

strncpy(new_cached->filename, filename, sizeof(new_cached->filename) - 1);
new_cached->filename[sizeof(new_cached->filename) - 1] = '\0';

new_cached->buffer = buffer;
new_cached->file_size = bytes_read;
new_cached->last_accessed = time(NULL);
new_cached->next = cache->files[index];

cache->files[index] = new_cached;
cache->cached_files_count++;

printf("Cached file %s of size %zu bytes \n", filename, bytes_read);

return buffer->data;
}

//to remove the cached file
file_cache_t* remove_cache_file(file_cache_t *cache, const char *filename){
    if(!cache) return NULL;
    unsigned int i = hash(filename);
    cached_file_t *current = cache->files[i];
    cached_file_t *prev = NULL;
    while(current){
        if (strcmp(current->filename, filename) == 0){
            if(prev){
                prev->next = current->next;
            }
            else{
                cache->files[i] = current->next;
            }
            release_buffer(cache->buffer_pool, current->buffer);
            free(current);
            cache->cached_files_count--;
            printf("Removed cached file: %s\n", filename);
            return cache;
        }
        prev = current;
        current = current->next;

    }
    printf("File not found in cache: %s\n",filename);
    return cache;
}
//LRU removal
file_cache_t* remove_lru_cached_file(file_cache_t *cache) {
    if (!cache) return NULL;
    cached_file_t *lru = NULL;
    cached_file_t *lru_prev = NULL;
    int lru_index = -1;
    for (int i = 0; i < CACHE_SIZE; i++) {
        cached_file_t *curr = cache->files[i];
        cached_file_t *prev = NULL;
        while (curr) {
            if (!lru || curr->last_accessed < lru->last_accessed) {
                lru = curr;
                lru_prev = prev;
                lru_index = i;
            }
            prev = curr;
            curr = curr->next;
        }
    }
    if (lru) {
        if (lru_prev) {
            lru_prev->next = lru->next;
        } else {
            cache->files[lru_index] = lru->next;
        }
        release_buffer(cache->buffer_pool, lru->buffer);
        free(lru);
        cache->cached_files_count--;
        printf("Removed LRU cached file\n");
    }
    return cache;
}

file_cache_t* remove_mru_cached_file(file_cache_t *cache){
    if(!cache) return NULL;
    cached_file_t *mru = NULL;
    cached_file_t *mru_prev = NULL;
    int mru_index = -1;
    for (int i = 0; i < CACHE_SIZE; i++) {
        cached_file_t *curr = cache->files[i];
        cached_file_t *prev = NULL;
        while (curr) {
            if (!mru || curr->last_accessed > mru->last_accessed) {
                mru = curr;
                mru_prev = prev;
                mru_index = i;
            }
            prev = curr;
            curr = curr->next;
        }
    }
    if (mru) {
        if (mru_prev) {
            mru_prev->next = mru->next;
        } else {
            cache->files[mru_index] = mru->next;
        }
        release_buffer(cache->buffer_pool, mru->buffer);
        free(mru);
        cache->cached_files_count--;
        printf("Removed MRU cached file\n");
    }
    return cache;
}