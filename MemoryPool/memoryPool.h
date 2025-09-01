#ifndef MEMORY_POOL
#define MEMORY_POOL

#include <stddef.h>

typedef struct Block {
    size_t size;           
    int free;              
    struct Block* next;    
} Block;

extern char* MemoryPool;    
extern size_t PoolSize;     
extern Block* FreeList;     
void PoolInit(size_t size);  
void* PoolAllocate(size_t size);
void PoolFree(void* ptr);
void PoolDestroy();

#endif
