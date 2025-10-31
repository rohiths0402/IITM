#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_PAIRS 100
#define KEY_LEN 64
#define VALUE_LEN 256

typedef struct {
    char key[KEY_LEN];
    char value[VALUE_LEN];
} kv_pair;

static kv_pair store[MAX_PAIRS];
static int count = 0;

void set_value(const char *key, const char *value) {
    for (int i = 0; i < count; i++) {
        if (strcmp(store[i].key, key) == 0) {
            strncpy(store[i].value, value, VALUE_LEN - 1);
            store[i].value[VALUE_LEN - 1] = '\0';
            return;
        }
    }
    if (count < MAX_PAIRS) {
        strncpy(store[count].key, key, KEY_LEN - 1);
        strncpy(store[count].value, value, VALUE_LEN - 1);
        store[count].key[KEY_LEN - 1] = '\0';
        store[count].value[VALUE_LEN - 1] = '\0';
        count++;
    }
}

const char* get_value(const char *key) {
    for (int i = 0; i < count; i++) {
        if (strcmp(store[i].key, key) == 0) {
            return store[i].value;
        }
    }
    return NULL;
}
