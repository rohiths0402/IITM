#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <time.h>
#include <unistd.h>
#include <fcntl.h>

#define BLOCK_SIZE 4096
#define BUFFER_POOL_SIZE 64
#define CACHE_BUCKETS 128
#define MAX_CACHE_SIZE (1024 * 1024)  

typedef struct buffer { 
    void *data;
    int block_id;
    int dirty;
    int ref_count;
    pthread_mutex_t mutex;
} buffer_t;

typedef struct buffer_pool {
    buffer_t *buffers;
    int *free_list;
    int free_count;
    int pool_size;
    pthread_mutex_t pool_mutex;
} buffer_pool_t;

typedef struct cache_entry {
    char *key;
    void *data;
    size_t size;
    time_t last_accessed;
    struct cache_entry *next;
    struct cache_entry *lru_prev;
    struct cache_entry *lru_next;
} cache_entry_t;

typedef struct cache {
    cache_entry_t **hash_table;
    cache_entry_t *lru_head;
    cache_entry_t *lru_tail;
    size_t bucket_count;
    size_t current_size;
    size_t max_size;
    pthread_mutex_t cache_mutex;
} cache_t;

typedef struct filesystem {
    int disk_fd;
    buffer_pool_t *buffer_pool;
    cache_t *cache;
    pthread_mutex_t fs_mutex;
} filesystem_t;

buffer_pool_t* create_buffer_pool(int size) {
    buffer_pool_t *pool = malloc(sizeof(buffer_pool_t));
    pool->buffers = malloc(sizeof(buffer_t) * size);
    pool->free_list = malloc(sizeof(int) * size);
    pool->pool_size = size;
    pool->free_count = size;
    pthread_mutex_init(&pool->pool_mutex, NULL);
    for (int i = 0; i < size; i++) {
        pool->buffers[i].data = aligned_alloc(BLOCK_SIZE, BLOCK_SIZE);
        pool->buffers[i].block_id = -1;
        pool->buffers[i].dirty = 0;
        pool->buffers[i].ref_count = 0;
        pthread_mutex_init(&pool->buffers[i].mutex, NULL);
        pool->free_list[i] = i;
    }
    return pool;
}

buffer_t* get_buffer(buffer_pool_t *pool) {
    pthread_mutex_lock(&pool->pool_mutex);
    
    if (pool->free_count == 0) {
        pthread_mutex_unlock(&pool->pool_mutex);
        return NULL; 
    }
    int buffer_idx = pool->free_list[--pool->free_count];
    buffer_t *buffer = &pool->buffers[buffer_idx];
    buffer->ref_count = 1;
    pthread_mutex_unlock(&pool->pool_mutex);
    return buffer;
}

void release_buffer(buffer_pool_t *pool, buffer_t *buffer) {
    pthread_mutex_lock(&pool->pool_mutex);
    pthread_mutex_lock(&buffer->mutex);
    buffer->ref_count--;
    if (buffer->ref_count == 0) {
        if (buffer->dirty) {
            printf("Writing dirty block %d to disk\n", buffer->block_id);
            buffer->dirty = 0;
        }
        buffer->block_id = -1;
        int buffer_idx = buffer - pool->buffers;
        pool->free_list[pool->free_count++] = buffer_idx;
    }
    pthread_mutex_unlock(&buffer->mutex);
    pthread_mutex_unlock(&pool->pool_mutex);
}

unsigned int hash_key(const char *key, size_t bucket_count) {
    unsigned int hash = 5381;
    while (*key) {
        hash = ((hash << 5) + hash) + *key++;
    }
    return hash % bucket_count;
}

cache_t* create_cache(size_t max_size, size_t bucket_count) {
    cache_t *cache = malloc(sizeof(cache_t));
    cache->hash_table = calloc(bucket_count, sizeof(cache_entry_t*));
    cache->bucket_count = bucket_count;
    cache->max_size = max_size;
    cache->current_size = 0;
    cache->lru_head = NULL;
    cache->lru_tail = NULL;
    pthread_mutex_init(&cache->cache_mutex, NULL);
    
    return cache;
}

void move_to_front(cache_t *cache, cache_entry_t *entry) {
    if (entry == cache->lru_head) return;
    
    if (entry->lru_prev) entry->lru_prev->lru_next = entry->lru_next;
    if (entry->lru_next) entry->lru_next->lru_prev = entry->lru_prev;
    if (entry == cache->lru_tail) cache->lru_tail = entry->lru_prev;
    
    entry->lru_prev = NULL;
    entry->lru_next = cache->lru_head;
    if (cache->lru_head) cache->lru_head->lru_prev = entry;
    cache->lru_head = entry;
    if (!cache->lru_tail) cache->lru_tail = entry;
}

void evict_lru(cache_t *cache) {
    if (!cache->lru_tail) return;
    cache_entry_t *victim = cache->lru_tail;
    cache->lru_tail = victim->lru_prev;
    if (cache->lru_tail) cache->lru_tail->lru_next = NULL;
    else cache->lru_head = NULL;
    unsigned int bucket = hash_key(victim->key, cache->bucket_count);
    cache_entry_t **current = &cache->hash_table[bucket];
    while (*current && *current != victim) {
        current = &(*current)->next;
    }
    if (*current) *current = victim->next;
    cache->current_size -= victim->size;
    free(victim->key);
    free(victim->data);
    free(victim);
}

void* cache_get(cache_t *cache, const char *key) {
    pthread_mutex_lock(&cache->cache_mutex);
    unsigned int bucket = hash_key(key, cache->bucket_count);
    cache_entry_t *entry = cache->hash_table[bucket]; 
    while (entry) {
        if (strcmp(entry->key, key) == 0) {
            entry->last_accessed = time(NULL);
            move_to_front(cache, entry);
            void *data = entry->data;
            pthread_mutex_unlock(&cache->cache_mutex);
            return data;
        }
        entry = entry->next;
    }
    pthread_mutex_unlock(&cache->cache_mutex);
    return NULL;
}

void cache_put(cache_t *cache, const char *key, void *data, size_t size) {
    pthread_mutex_lock(&cache->cache_mutex);
    while (cache->current_size + size > cache->max_size) {
        evict_lru(cache);
    }
    cache_entry_t *entry = malloc(sizeof(cache_entry_t));
    entry->key = strdup(key);
    entry->data = malloc(size);
    memcpy(entry->data, data, size);
    entry->size = size;
    entry->last_accessed = time(NULL);
    unsigned int bucket = hash_key(key, cache->bucket_count);
    entry->next = cache->hash_table[bucket];
    cache->hash_table[bucket] = entry;
    entry->lru_prev = NULL;
    entry->lru_next = cache->lru_head;
    if (cache->lru_head) cache->lru_head->lru_prev = entry;
    cache->lru_head = entry;
    if (!cache->lru_tail) cache->lru_tail = entry;
    cache->current_size += size;
    pthread_mutex_unlock(&cache->cache_mutex);
}

filesystem_t* create_filesystem(const char *disk_file) {
    filesystem_t *fs = malloc(sizeof(filesystem_t));
    fs->disk_fd = open(disk_file, O_RDWR | O_CREAT, 0644);
    if (fs->disk_fd == -1) {
        perror("Failed to open disk file");
        free(fs);
        return NULL;
    }
    fs->buffer_pool = create_buffer_pool(BUFFER_POOL_SIZE);
    fs->cache = create_cache(MAX_CACHE_SIZE, CACHE_BUCKETS);
    pthread_mutex_init(&fs->fs_mutex, NULL);
    return fs;
}

int read_block(filesystem_t *fs, int block_id, void *buffer) {
    char cache_key[32];
    snprintf(cache_key, sizeof(cache_key), "block_%d", block_id);
    
    void *cached_data = cache_get(fs->cache, cache_key);
    if (cached_data) {
        printf("Cache hit for block %d\n", block_id);
        memcpy(buffer, cached_data, BLOCK_SIZE);
        return 0;
    }
    printf("Cache miss for block %d\n", block_id);
    buffer_t *buf = get_buffer(fs->buffer_pool);
    if (!buf) {
        printf("No free buffers available\n");
        return -1;
    }
    pthread_mutex_lock(&buf->mutex);
    lseek(fs->disk_fd, block_id * BLOCK_SIZE, SEEK_SET);
    ssize_t bytes_read = read(fs->disk_fd, buf->data, BLOCK_SIZE);
    if (bytes_read != BLOCK_SIZE) {
        pthread_mutex_unlock(&buf->mutex);
        release_buffer(fs->buffer_pool, buf);
        return -1;
    }
    buf->block_id = block_id;
    memcpy(buffer, buf->data, BLOCK_SIZE);
    pthread_mutex_unlock(&buf->mutex);
    cache_put(fs->cache, cache_key, buf->data, BLOCK_SIZE);
    release_buffer(fs->buffer_pool, buf);
    return 0;
}

int write_block(filesystem_t *fs, int block_id, const void *buffer) {
    buffer_t *buf = get_buffer(fs->buffer_pool);
    if (!buf) {
        return -1;
    }
    pthread_mutex_lock(&buf->mutex);
    memcpy(buf->data, buffer, BLOCK_SIZE);
    buf->block_id = block_id;
    buf->dirty = 1;
    pthread_mutex_unlock(&buf->mutex);
    char cache_key[32];
    snprintf(cache_key, sizeof(cache_key), "block_%d", block_id);
    cache_put(fs->cache, cache_key, (void*)buffer, BLOCK_SIZE);
    lseek(fs->disk_fd, block_id * BLOCK_SIZE, SEEK_SET);
    write(fs->disk_fd, buffer, BLOCK_SIZE);
    release_buffer(fs->buffer_pool, buf);
    printf("Written block %d to disk\n", block_id);
    return 0;
}

int main() {
    
    filesystem_t *fs = create_filesystem("test_disk.img");
    if (!fs) return 1;
    char write_data[BLOCK_SIZE];
    char read_data[BLOCK_SIZE];
    for (int i = 0; i < BLOCK_SIZE; i++) {
        write_data[i] = (char)(i % 256);
    }
    printf("=== File System with Buffer Pool and Cache Demo ===\n\n");
    printf("Writing blocks 0, 1, 2...\n");
    write_block(fs, 0, write_data);
    write_block(fs, 1, write_data);
    write_block(fs, 2, write_data);
    printf("\nReading blocks (should hit cache):\n");
    read_block(fs, 0, read_data);   
    read_block(fs, 1, read_data);   
    read_block(fs, 2, read_data);   
    printf("\nReading new block (should miss cache):\n");
    read_block(fs, 5, read_data); 
    printf("\nReading block 5 again (should hit cache):\n");
    read_block(fs, 5, read_data);
    close(fs->disk_fd);
    
    return 0;
}