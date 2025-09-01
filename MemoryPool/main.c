#include <stdio.h>
#include "memoryPool.c"

int main(){
    size_t user_size;
    printf("Enter memory pool size: ");
    scanf("%zu", &user_size);
    PoolInit(user_size);
    char* data = (char*)PoolAllocate(100);
    if (data){
        printf("Allocated 100 bytes successfully.\n");
    }
    PoolFree(data);
    printf("Memory freed.\n");
    PoolDestroy();
    printf("Memory pool destroyed.\n");
    return 0;

}