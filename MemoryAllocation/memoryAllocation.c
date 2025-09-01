#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct MemoryNode{
    void *address;
    size_t size;
    int is_freed;
    struct MemoryNode *next;
} MemoryNode;

MemoryNode *head = NULL;
FILE *logFile = NULL;

int LogEvent(const char *action, void *address, size_t size){
    if (!logFile){
        logFile = fopen("memoryLog.txt", "a");
        if (!logFile){
            fprintf(stderr, "Error opening log file.\n");
            return 1;
        }
        fprintf(logFile, "---------Memory log---------\n");
    }
    fprintf(logFile, "[%-7s] Address = %p, Size = %zu bytes\n", action, address, size);
    fflush(logFile);
    return 0;
}

int AddAllocation(void *address, size_t size){
    MemoryNode *newNode = malloc(sizeof(MemoryNode));
    if (!newNode){
        LogEvent("TRACKER ALLOC FAILED", NULL, sizeof(MemoryNode));
        return 1;
    }
    newNode->address = address;
    newNode->size = size;
    newNode->is_freed = 0;
    newNode->next = head;
    head = newNode;
    return 0;
}

MemoryNode *FindeValue(void *address){
    MemoryNode *curr = head;
    while (curr){
        if (curr->address == address) return curr;
        curr = curr->next;
    }
    return NULL;
}

void *TrackMalloc(size_t size){
    if (size == 0){
        LogEvent("MALLOCZERO", NULL, 0);
        return NULL;
    }
    void *ptr = malloc(size);
    if (ptr){
        AddAllocation(ptr, size);
        LogEvent("MALLOC", ptr, size);
    } else{
        LogEvent("MALLOC FAILED", NULL, size);
    }
    return ptr;
}

void *TrackCalloc(size_t num, size_t size){
    if (num == 0 || size == 0){
        LogEvent("CALLOC ZERO", NULL, 0);
        return NULL;
    }
    void *ptr = calloc(num, size);
    if (ptr){
        AddAllocation(ptr, num * size);
        LogEvent("CALLOC", ptr, num * size);
    } else{
        LogEvent("CALLOC FAILED", NULL, num * size);
    }
    return ptr;
}

void *TrackRealloc(void *ptr, size_t size){
    if (size == 0){
        LogEvent("REALLOC ZERO", ptr, 0);
        return NULL;
    }

    void *new_ptr = realloc(ptr, size);
    if (new_ptr){
        MemoryNode *node = FindeValue(ptr);
        if (node){
            node->is_freed = 1;
            AddAllocation(new_ptr, size);
            LogEvent("REALLOC", new_ptr, size);
        } else{
            LogEvent("REALLOC UNTRACKED", new_ptr, size);
        }
    } else{
        LogEvent("REALLOC FAILED", ptr, size);
    }
    return new_ptr;
}

int TrackFree(void *ptr){
    if (!ptr){
        LogEvent("FREE NULL", NULL, 0);
        return 1;
    }
    MemoryNode *node = FindeValue(ptr);
    if (node && !node->is_freed){
        node->is_freed = 1;
        LogEvent("FREE", ptr, node->size);
        free(ptr);
        return 0;
    } else{
        LogEvent("FREE UNTRACKED", ptr, 0);
        return 1;
    }
}

int FreedMemory(){
    if (!logFile) return 1;
    fprintf(logFile, "\n--- Freed Memory Blocks ---\n");
    MemoryNode *curr = head;
    while (curr){
        if (curr->is_freed){
            fprintf(logFile, "Address: %p, Size: %zu bytes\n", curr->address, curr->size);
        }
        curr = curr->next;
    }
    return 0;
}

int MemoryLeaks(){
    if (!logFile) return 1;
    fprintf(logFile, "\n-----Memory leak------\n");
    size_t total_leaked = 0;
    MemoryNode *curr = head;
    while (curr){
        if (!curr->is_freed){
            fprintf(logFile, "Address: %p, Size: %zu bytes\n", curr->address, curr->size);
            total_leaked += curr->size;
        }
        curr = curr->next;
    }
    fprintf(logFile, "Total leaked memory: %zu bytes\n", total_leaked);
    return 0;
}

int TableView(){
    if (!logFile) return 1;
    fprintf(logFile, "\n--- Active Allocations (Table) ---\n");
    fprintf(logFile, "| %-17s | %-12s |\n", "Address", "Size (bytes)");
    fprintf(logFile, "|-------------------|--------------|\n");

    MemoryNode *curr = head;
    while (curr){
        if (!curr->is_freed){
            fprintf(logFile, "| %-17p | %-12zu |\n", curr->address, curr->size);
        }
        curr = curr->next;
    }
    return 0;
}

int LinkedListView(){
    if (!logFile) return 1;
    fprintf(logFile, "\n--- Active Allocations (Linked List) ---\n");
    MemoryNode *curr = head;
    while (curr){
        if (!curr->is_freed){
            fprintf(logFile, "[Address: %p | Size: %zu bytes] -> ", curr->address, curr->size);
        }
        curr = curr->next;
    }
    fprintf(logFile, "NULL\n");
    return 0;
}
int CleanupTracker(){
    if (!logFile) return 1;
    FreedMemory();
    MemoryLeaks();
    TableView();
    LinkedListView();
    MemoryNode *curr = head;
    while (curr){
        MemoryNode *temp = curr;
        curr = curr->next;
        free(temp);
    }
    head = NULL;
    fclose(logFile);
    logFile = NULL;
    return 0;
}

int main(){
    int choice;
    void *ptrs[100] ={NULL}; 
    int index = 0;
    while (1){
        printf("\nMemory Tracker Menu:\n");
        printf("1. Malloc\n");
        printf("2. Calloc\n");
        printf("3. Realloc\n");
        printf("4. Free\n");
        printf("5. Show Table View\n");
        printf("6. Show Linked List View\n");
        printf("7. Cleanup and Exit\n");
        printf("Enter your choice: ");
        scanf("%d", &choice);
        if (choice == 1){
            size_t size;
            printf("Enter size to malloc: ");
            scanf("%zu", &size);
            ptrs[index++] = TrackMalloc(size);
        } else if (choice == 2){
            size_t num, size;
            printf("Enter number of elements: ");
            scanf("%zu", &num);
            printf("Enter size of each element: ");
            scanf("%zu", &size);
            ptrs[index++] = TrackCalloc(num, size);
        } else if (choice == 3){
            int i;
            size_t newSize;
            printf("Enter index of pointer to realloc (0 to %d): ", index - 1);
            scanf("%d", &i);
            if (i >= 0 && i < index && ptrs[i]){
                printf("Enter new size: ");
                scanf("%zu", &newSize);
                ptrs[i] = TrackRealloc(ptrs[i], newSize);
            } else{
                printf("Invalid index.\n");
            }
        } else if (choice == 4){
            int i;
            printf("Enter index of pointer to free (0 to %d): ", index - 1);
            scanf("%d", &i);
            if (i >= 0 && i < index && ptrs[i]){
                TrackFree(ptrs[i]);
                ptrs[i] = NULL;
            } else{
                printf("Invalid index.\n");
            }
        } else if (choice == 5){
            TableView();
        } else if (choice == 6){
            LinkedListView();
        } else if (choice == 7){
            CleanupTracker();
            printf("Cleanup done. Exiting...\n");
            break;
        } else{
            printf("Invalid choice. Try again.\n");
        }
    }

    return 0;
}