#ifndef SLAB_ALLOCATOR_H
#define SLAB_ALLOCATOR_H

void initSlabCaches();
void cleanupSlabCaches();
void createDataPage();     
void manageLock();          
void logTransaction();  

#endif 