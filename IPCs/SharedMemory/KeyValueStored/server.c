#define _GNU_SOURCE
#include <sys/socket.h>
#include <sys/un.h>
#include <pthread.h>
#include <signal.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define SOCKET_PATH "/tmp/uds-demo.sock"
#define BACKLOG 5
#define BUF_SIZE 512
#define MAX_ITEMS 128
#define KEY_LEN 64
#define VAL_LEN 256

typedef struct{
    char key[KEY_LEN];
    char val[VAL_LEN];
} KVPair;

static KVPair store[MAX_ITEMS];
static int store_count = 0;
pthread_mutex_t store_lock = PTHREAD_MUTEX_INITIALIZER;

void die(const char *msg){
    perror(msg);
    exit(EXIT_FAILURE);
}

void cleanup(void){
    unlink(SOCKET_PATH);
}

void on_signal(int sig){
   (void)sig;
    exit(0);
}

void set_value(const char *k, const char *v){
    pthread_mutex_lock(&store_lock);
    for(int i = 0; i < store_count; i++){
        if(strcmp(store[i].key, k) == 0){
            strncpy(store[i].val, v, VAL_LEN - 1);
            pthread_mutex_unlock(&store_lock);
            return;
        }
    }
    if(store_count < MAX_ITEMS){
        strncpy(store[store_count].key, k, KEY_LEN - 1);
        strncpy(store[store_count].val, v, VAL_LEN - 1);
        store_count++;
    }
    pthread_mutex_unlock(&store_lock);
}

const char* get_value(const char *k){
    for(int i = 0; i < store_count; i++){
        if(strcmp(store[i].key, k) == 0)
            return store[i].val;
    }
    return NULL;
}

void* client_handler(void *arg){
    int client_fd = *(int*)arg;
    free(arg);

    char buf[BUF_SIZE];
    write(client_fd, "Mini KV Store ready\n", 20);

    while(1){
        ssize_t n = read(client_fd, buf, sizeof(buf) - 1);
        if(n <= 0){
            break;
        }
        buf[n] = '\0';
        buf[strcspn(buf, "\r\n")] = '\0';
        if(strncmp(buf, "SET ", 4) == 0){
            char key[KEY_LEN], val[VAL_LEN];
            if(sscanf(buf + 4, "%63s %255[^\n]", key, val) == 2){
                set_value(key, val);
                write(client_fd, "OK\n", 3);
            } else{
                write(client_fd, "ERR usage: SET <key> <value>\n", 29);
            }
        } 
        else if(strncmp(buf, "GET ", 4) == 0){
            char key[KEY_LEN];
            if(sscanf(buf + 4, "%63s", key) == 1){
                const char *v = get_value(key);
                if(v){
                    dprintf(client_fd, "VALUE %s\n", v);
                } else{
                    write(client_fd, "NOTFOUND\n", 9);
                }
            }
            else{
                write(client_fd, "ERR usage: GET <key>\n", 21);
            }
        }
        else if(strcmp(buf, "EXIT") == 0){
            write(client_fd, "BYE\n", 4);
            break;
        }
        else{
            write(client_fd, "ERR unknown command\n", 20);
        }
    }
    close(client_fd);
    return NULL;
}

int main(void){
    atexit(cleanup);
    signal(SIGINT, on_signal);
    signal(SIGTERM, on_signal);
    int listen_fd = socket(AF_UNIX, SOCK_STREAM, 0);
    if(listen_fd == -1) die("socket");
    struct sockaddr_un addr ={0};
    addr.sun_family = AF_UNIX;
    strncpy(addr.sun_path, SOCKET_PATH, sizeof(addr.sun_path) - 1);
    unlink(SOCKET_PATH);
    if(bind(listen_fd,(struct sockaddr*)&addr, sizeof(addr)) == -1){
        die("bind");
    }
    if(listen(listen_fd, BACKLOG) == -1){
        die("listen");
    }
    printf("[Server] Listening on %s\n", SOCKET_PATH);
    while(1){
        int *client_fd = malloc(sizeof(int));
        *client_fd = accept(listen_fd, NULL, NULL);
        if(*client_fd == -1){
            free(client_fd);
            if(errno == EINTR) continue;
            die("accept");
        }
        pthread_t tid;
        pthread_create(&tid, NULL, client_handler, client_fd);
        pthread_detach(tid); 
    }

    return 0;
}