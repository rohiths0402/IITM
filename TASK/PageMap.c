#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#define ROWS 5000
#define COLS 5000
#define PAGE_SIZE 4096

int main(void){
    size_t totalElements =(size_t)ROWS * COLS;
    size_t totalBytes = totalElements * sizeof(int);
    int *array = malloc(totalBytes);
    if(!array){
        perror("Memory allocation failed");
        return 1;
    }
    printf("Allocated %zu bytes(%zu elements)\n", totalBytes, totalElements);
    printf("Base address: %p\n\n",(void *)array);
    uintptr_t baseAddress =(uintptr_t)array;
    uintptr_t lastAddress = baseAddress + totalBytes - 1;
    uintptr_t pageStart = baseAddress & ~(PAGE_SIZE - 1);
    uintptr_t pageEnd = lastAddress & ~(PAGE_SIZE - 1);
    printf("Page size: %d bytes\n", PAGE_SIZE);
    printf("Pages covered: %zu\n\n",(pageEnd - pageStart) / PAGE_SIZE + 1);
    printf("---Page start addresses---\n");
    for(uintptr_t addr = pageStart; addr <= pageEnd; addr += PAGE_SIZE){
        printf("%p\n",(void *)addr);
    }
    printf("----Page mapping example----\n");
    for(int i = 0; i < 5; i++){
        size_t row = i * 1000;
        size_t col = i * 1000;
        size_t index = row * COLS + col;
        uintptr_t elem_addr =(uintptr_t)&array[index];
        uintptr_t elem_page = elem_addr & ~(PAGE_SIZE - 1);
        printf("Element[%zu][%zu] @ %p → Page start: %p\n", row, col,(void *)elem_addr,(void *)elem_page);
    }

    free(array);
    return 0;
}
