#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>

#define NUM_BUCKETS 16  

typedef struct Node{
    char* key;
    int value;
    struct Node* next;
} Node;

typedef struct{
    Node* head;
    pthread_mutex_t lock;
} Bucket;

typedef struct{
    Bucket buckets[NUM_BUCKETS];
} ConcurrentHashMap;

unsigned int hash(const char* key){
    unsigned long h = 5381;
    int c;
    while((c = *key++)){
        h =((h << 5) + h) + c; 
    }
    return h % NUM_BUCKETS;
}

void hashmapInit(ConcurrentHashMap* map){
    for(int i = 0; i < NUM_BUCKETS; i++){
        map->buckets[i].head = NULL;
        pthread_mutex_init(&map->buckets[i].lock, NULL);
    }
}

void hashmapInsert(ConcurrentHashMap* map, const char* key, int value){
    unsigned int idx = hash(key);
    Bucket* bucket = &map->buckets[idx];
    pthread_mutex_lock(&bucket->lock);
    Node* curr = bucket->head;
    while(curr){
        if(strcmp(curr->key, key) == 0){
            curr->value = value;
            pthread_mutex_unlock(&bucket->lock);
            return;
        }
        curr = curr->next;
    }
    Node* newNoe = malloc(sizeof(Node));
    newNoe->key = strdup(key);
    newNoe->value = value;
    newNoe->next = bucket->head;
    bucket->head = newNoe;
    pthread_mutex_unlock(&bucket->lock);
}

int hashmapLookup(ConcurrentHashMap* map, const char* key, int* found){
    unsigned int idx = hash(key);
    Bucket* bucket = &map->buckets[idx];
    pthread_mutex_lock(&bucket->lock);
    Node* curr = bucket->head;
    while(curr){
        if(strcmp(curr->key, key) == 0){
            int value = curr->value;
            pthread_mutex_unlock(&bucket->lock);
            *found = 1;
            return value;
        }
        curr = curr->next;
    }
    pthread_mutex_unlock(&bucket->lock);
    *found = 0;
    return -1;
}

void hashmapDestroy(ConcurrentHashMap* map){
    for(int i = 0; i < NUM_BUCKETS; i++){
        pthread_mutex_lock(&map->buckets[i].lock);
        Node* curr = map->buckets[i].head;
        while(curr){
            Node* temp = curr;
            curr = curr->next;
            free(temp->key);
            free(temp);
        }
        pthread_mutex_unlock(&map->buckets[i].lock);
        pthread_mutex_destroy(&map->buckets[i].lock);
    }
}

ConcurrentHashMap map;
void* writer_thread(void* arg){
    char key[32];
    for(int i = 0; i < 10; i++){
        snprintf(key, sizeof(key), "key_%d", i);
        hashmapInsert(&map, key, i * 100);
        printf("Writer inserted: %s -> %d\n", key, i * 100);
    }
    return NULL;
}

void* reader_thread(void* arg){
    char key[32];
    for(int i = 0; i < 10; i++){
        snprintf(key, sizeof(key), "key_%d", i);
        int found;
        int value = hashmapLookup(&map, key, &found);
        if(found){
            printf("Reader found: %s -> %d\n", key, value);
        } else{
            printf("Reader did not find: %s\n", key);
        }
    }
    return NULL;
}

int main(){
    pthread_t t1, t2;
    hashmapInit(&map);
    pthread_create(&t1, NULL, writer_thread, NULL);
    pthread_create(&t2, NULL, reader_thread, NULL);
    pthread_join(t1, NULL);
    pthread_join(t2, NULL);
    hashmapDestroy(&map);
    return 0;
}

