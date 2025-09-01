    #include "memoryPool.h"
    #include <stdio.h>
    #include <stdlib.h>
    

    void PoolInit(size_t size){
       MemoryPool = (char*)malloc(size);
        if (!MemoryPool) {
            printf("Memory allocation failed!\n");
            exit(1);
        }
        PoolSize = size;
        FreeList = (Block*)MemoryPool;
        FreeList->size = size - sizeof(Block);
        FreeList->free = 1;
        FreeList->next = NULL;
        printf("Memory pool of %zu bytes initialized.\n", PoolSize);
    }

    static void SplitBlock(Block* block, size_t size){
        Block* NewBlock = (Block*)((char*)block + sizeof(Block) + size);
        NewBlock->size = block->size - size - sizeof(Block);
        NewBlock->free = 1;
        NewBlock->next = block->next;
        block->size = size;
        block->free = 0;
        block->next = NewBlock;
    }

    void* PoolAllocate(size_t size){
        Block* curr = FreeList;
        while (curr != NULL) {
            if (curr->free && curr->size >= size) {
                if (curr->size > size + sizeof(Block)){
                    SplitBlock(curr, size);
                } else{
                    curr->free = 0; 
                }
                printf("Allocated %zu bytes at %p\n", size, (void*)((char*)curr + sizeof(Block)));
                return (char*)curr + sizeof(Block);
            }
            curr = curr->next;
        }
        printf("Allocation of %zu bytes failed: Not enough memory\n", size);
        return NULL; 
    }

    static void MergeBlock(){
        Block* curr = FreeList;
        while (curr != NULL && curr->next != NULL){
            if (curr->free && curr->next->free){
                curr->size += sizeof(Block) + curr->next->size;
                curr->next = curr->next->next;
            } else {
                curr = curr->next;
            }
        }
    }

    void PoolFree(void* ptr){
        if (ptr == NULL) return;
        Block* block_ptr = (Block*)((char*)ptr - sizeof(Block));
        block_ptr->free = 1;
        printf("Freed %zu bytes at %p\n", block_ptr->size, ptr);
        MergeBlock();
    }

void PoolDestroy() {
        free(MemoryPool);
        MemoryPool = NULL;
        FreeList = NULL;
        PoolSize = 0;
        printf("Memory pool destroyed.\n");
    }