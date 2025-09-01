#ifndef HEAP_H
#define HEAP_H

#include <stddef.h>  

void* HeapAllocate(size_t size, const char* tag);
void  HeapFree(void* ptr);
void  HeapLeaks();

#endif