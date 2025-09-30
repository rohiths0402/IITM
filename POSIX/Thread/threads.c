#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>
#include <time.h>

#define MAX_KEY 64
#define MAX_VAL 256
#define MAX_OPS 100
#define MAX_LOGS 1000
#define MAX_THREADS 5

typedef struct Version{
    char value[MAX_VAL];
    time_t timestamp;
    struct Version* next;
}Version;

typedef struct KVNode{
    char key[MAX_KEY];
    Version* versions;
    pthread_rwlock_t lock;
    struct KVNode* next;
}KVNode;

typedef struct Log{
    int tid;
    char op[8];
    char key[MAX_KEY];
    char value[MAX_VAL];
    time_t ts;
}Log;

typedef struct Transaction{
    int tid;
    time_t start_time;
    Log ops[MAX_OPS];
    int op_count;
}Transaction;

KVNode* store = NULL;
pthread_mutex_t store_lock = PTHREAD_MUTEX_INITIALIZER;
Log logs[MAX_LOGS];
int log_index = 0;
int tid_counter = 1;

KVNode* get_node(const char* key){
    KVNode* cur = store;
    while(cur){
        if(strcmp(cur->key, key) == 0) return cur;
        cur = cur->next;
    }
    KVNode* node = malloc(sizeof(KVNode));
    strcpy(node->key, key);
    node->versions = NULL;
    pthread_rwlock_init(&node->lock, NULL);
    node->next = store;
    store = node;
    return node;
}

char* kv_get(Transaction* tx, const char* key){
    KVNode* node = get_node(key);
    pthread_rwlock_rdlock(&node->lock);
    Version* ver = node->versions;
    char* res = NULL;
    while(ver){
        if(ver->timestamp <= tx->start_time && strlen(ver->value) > 0){
            res = ver->value;
            break;
        }
        ver = ver->next;
    }
    pthread_rwlock_unlock(&node->lock);
    return res;
}

void kv_set(Transaction* tx, const char* key, const char* value){
    KVNode* node = get_node(key);
    pthread_rwlock_wrlock(&node->lock);
    Version* ver = malloc(sizeof(Version));
    strcpy(ver->value, value);
    ver->timestamp = time(NULL);
    ver->next = node->versions;
    node->versions = ver;
    if(tx->op_count < MAX_OPS){
        tx->ops[tx->op_count].tid = tx->tid;
        strcpy(tx->ops[tx->op_count].op, "SET");
        strcpy(tx->ops[tx->op_count].key, key);
        strcpy(tx->ops[tx->op_count].value, value);
        tx->ops[tx->op_count].ts = ver->timestamp;
        tx->op_count++;
    }
    pthread_rwlock_unlock(&node->lock);
}

void kv_delete(Transaction* tx, const char* key){
    KVNode* node = get_node(key);
    pthread_rwlock_wrlock(&node->lock);
    Version* ver = malloc(sizeof(Version));
    strcpy(ver->value, "");
    ver->timestamp = time(NULL);
    ver->next = node->versions;
    node->versions = ver;
    if(tx->op_count < MAX_OPS){
        tx->ops[tx->op_count].tid = tx->tid;
        strcpy(tx->ops[tx->op_count].op, "DEL");
        strcpy(tx->ops[tx->op_count].key, key);
        strcpy(tx->ops[tx->op_count].value, "");
        tx->ops[tx->op_count].ts = ver->timestamp;
        tx->op_count++;
    }
    pthread_rwlock_unlock(&node->lock);
}

void commit(Transaction* tx){
    pthread_mutex_lock(&store_lock);
    for(int i = 0; i < tx->op_count; i++){
        logs[log_index++] = tx->ops[i];
    }
    pthread_mutex_unlock(&store_lock);
    printf("Transaction %d committed with %d operations.\n", tx->tid, tx->op_count);
}

void rollback(Transaction* tx){
    for(int i = tx->op_count - 1; i >= 0; i--){
        KVNode* node = get_node(tx->ops[i].key);
        pthread_rwlock_wrlock(&node->lock);
        if(node->versions){
            Version* tmp = node->versions;
            node->versions = node->versions->next;
            free(tmp);
        }
        pthread_rwlock_unlock(&node->lock);
    }
    printf("Transaction %d rolled back.\n", tx->tid);
}

Transaction* begin_transaction(){
    Transaction* tx = malloc(sizeof(Transaction));
    tx->tid = tid_counter++;
    tx->start_time = time(NULL);
    tx->op_count = 0;
    printf("Transaction %d started.\n", tx->tid);
    return tx;
}

void print_store(Transaction* tx){
    printf("\nStore snapshot at tx %d:\n", tx->tid);
    KVNode* cur = store;
    while(cur){
        pthread_rwlock_rdlock(&cur->lock);
        Version* ver = cur->versions;
        while(ver){
            if(ver->timestamp <= tx->start_time && strlen(ver->value) > 0){
                printf("  %s -> %s\n", cur->key, ver->value);
                break;
            }
            ver = ver->next;
        }
        pthread_rwlock_unlock(&cur->lock);
        cur = cur->next;
    }
}

void print_logs(){
    printf("\nAll operation logs:\n");
    for(int i = 0; i < log_index; i++){
        printf("T%d: %s %s %s(ts=%ld)\n",
               logs[i].tid, logs[i].op, logs[i].key,
               logs[i].value, logs[i].ts);
    }
}

void list_keys(){
    printf("\nKeys in store:\n");
    KVNode* cur = store;
    while(cur){
        printf("  %s\n", cur->key);
        cur = cur->next;
    }
}

void list_versions(const char* key){
    KVNode* node = get_node(key);
    pthread_rwlock_rdlock(&node->lock);
    Version* ver = node->versions;
    printf("Versions of %s:\n", key);
    while(ver){
        printf("  %s(ts=%ld)\n", ver->value, ver->timestamp);
        ver = ver->next;
    }
    pthread_rwlock_unlock(&node->lock);
}

void* thread_demo(void* arg){
    Transaction* tx = begin_transaction();
    kv_set(tx, "x", "100");
    kv_set(tx, "y", "200");
    kv_set(tx, "z", "300");
    kv_delete(tx, "y");
    printf("x in tx %d: %s\n", tx->tid, kv_get(tx, "x"));
    commit(tx);
    free(tx);
    return NULL;
}

void console_demo(){
    char cmd[16], key[MAX_KEY], val[MAX_VAL];
    Transaction* tx = begin_transaction();
    while(1){
        printf("\nEnter cmd(get/set/del/commit/rollback/exit): ");
        scanf("%s", cmd);
        if(strcmp(cmd, "exit") == 0) break;
        else if(strcmp(cmd, "get") == 0){
            scanf("%s", key);
            char* v = kv_get(tx, key);
            printf("%s -> %s\n", key, v ? v : "NULL");
        }else if(strcmp(cmd, "set") == 0){
            scanf("%s %s", key, val);
            kv_set(tx, key, val);
        }else if(strcmp(cmd, "del") == 0){
            scanf("%s", key);
            kv_delete(tx, key);
        }else if(strcmp(cmd, "commit") == 0){
            commit(tx);
            free(tx);
            tx = begin_transaction();
        }else if(strcmp(cmd, "rollback") == 0){
            rollback(tx);
            free(tx);
            tx = begin_transaction();
        }else if(strcmp(cmd, "list") == 0){
            list_keys();
        }else if(strcmp(cmd, "vers") == 0){
            scanf("%s", key);
            list_versions(key);
        }
    }
    free(tx);
}

int main(){
    pthread_t threads[MAX_THREADS];
    for(int i = 0; i < MAX_THREADS; i++){
        pthread_create(&threads[i], NULL, thread_demo, NULL);
    }
    for(int i = 0; i < MAX_THREADS; i++){
        pthread_join(threads[i], NULL);
    }
    console_demo();
    print_logs();
    return 0;
}
