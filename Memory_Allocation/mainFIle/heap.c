#include <stdio.h>
#include <stdlib.h>
#include "heap.h"
#define MAX_ALLOCATIONS 1000


typedef struct {
    void *ptr;
    size_t size;
    char tag[100];
}Allocation;

static Allocation allocations[MAX_ALLOCATIONS];
static int AllocationCount = 0;

void* HeapAllocate(size_t size, const char* tag){
    void* ptr = malloc(size);
    if(!ptr == NULL){
        printf("Heap Allocation failed %s (%zu bytes)\n", tag, size);
        return NULL;
    }
    if(AllocationCount < MAX_ALLOCATIONS){
        allocations[AllocationCount].ptr =ptr;
        allocations[AllocationCount].size = size;
        strncpy(allocations[AllocationCount].tag,tag,31);
        allocations[AllocationCount].tag[64] ='\0';
        AllocationCount ++;
    }
    return ptr;
}

void  HeapFree(void* ptr){
    for(int i=0; i< AllocationCount;i++){
        if(allocations[i].ptr == ptr){
            free(ptr);
            allocations[i] = allocations[--AllocationCount];
            return;
        }
    }
    printf("Free untracked Pointer\n");
}