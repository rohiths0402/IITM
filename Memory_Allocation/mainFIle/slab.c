#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "slab.h"
#include "heap.h"
#define MAX_OBJECTS 100

typedef struct{
    int page_id;
    char status;
} PageHeader;


typedef struct {
    int lock_id;
    int is_acquired;
} Lock;

typedef struct {
    int tx_id;
    char operation[20];
} TxLog;

static PageHeader* PageCache[MAX_OBJECTS];
static Lock* LockCache[MAX_OBJECTS];
static TxLog* TxCache[MAX_OBJECTS];
static int pageCount;
static int lockCounnt;
static int TxCount;

void initSlabCaches(){
    pageCount = lockCounnt = TxCount = 0;
}

void cleanupSlabCaches(){
    for(int i=0;i<pageCount;i++){
        HeapFree(PageCache[i]);
    }
    for(int i=0;i<lockCounnt;i++){
        HeapFree(LockCache[i]);
    }
    for(int i=0;i<TxCount;i++){
        HeapFree(TxCache[i]);
    }
}

void createDataPage(){
    if (pageCount >= MAX_OBJECTS) {
        printf("Page cache full.\n");
        return;
    }
    PageHeader* page = (PageHeader*)heap_alloc(sizeof(PageHeader), "PageHeader");
    if (!page) return;
    printf("Enter Page ID: \n");
    scanf("%d", &page->page_id);
    printf("Enter Status (A/I): \n");
    scanf(" %c", &page->status);
    PageCache[pageCount++] = page;
    printf("PageHeader created\n");

} 

void manageLock(){
    if(lockCounnt >= MAX_OBJECTS){
        printf("Lock Cache full\n");
        return;
    }
    Lock* lock = (Lock*)heap_alloc(sizeof(Lock), "Lock");
    if (!lock) return;
    printf("Enter Lock ID: ");
    scanf("%d", &lock->lock_id);
    printf("Acquire lock? (1/0): ");
    scanf("%d", &lock->is_acquired);
    LockCache[lockCounnt++] = lock;
    printf("Lock object created.\n");

}

void logTransaction(){
    if(TxCount >= MAX_OBJECTS){
        printf("Cache Full \n");
        return;
    }
    // TxLog * txlog = (TxLog*)heap_alloc()
}