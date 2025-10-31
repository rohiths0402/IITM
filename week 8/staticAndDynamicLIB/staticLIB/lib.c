#include "lib.h"
#include <string.h>
#include <pthread.h>

#define MAX_ITEMS 128
#define KEY_LEN 64
#define VAL_LEN 256

typedef struct {
    char key[KEY_LEN];
    char val[VAL_LEN];
} KVPair;

static KVPair store[MAX_ITEMS];
static int store_count = 0;
static pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;

void set_value(const char *key, const char *val) {
    pthread_mutex_lock(&lock);
    for (int i = 0; i < store_count; i++) {
        if (strcmp(store[i].key, key) == 0) {
            strncpy(store[i].val, val, VAL_LEN - 1);
            pthread_mutex_unlock(&lock);
            return;
        }
    }
    if (store_count < MAX_ITEMS) {
        strncpy(store[store_count].key, key, KEY_LEN - 1);
        strncpy(store[store_count].val, val, VAL_LEN - 1);
        store_count++;
    }
    pthread_mutex_unlock(&lock);
}

const char* get_value(const char *key) {
    pthread_mutex_lock(&lock);
    for (int i = 0; i < store_count; i++) {
        if (strcmp(store[i].key, key) == 0) {
            pthread_mutex_unlock(&lock);
            return store[i].val;
        }
    }
    pthread_mutex_unlock(&lock);
    return NULL;
}
