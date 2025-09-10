#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>

#define QUEUE_SIZE 8
#define MAX_CLIENTS 3 
#define WORKER_COUNT 4
#define REQUESTS_PER_CLIENT 5
#define MAX_TEXT 64

typedef struct{
    int id;
    char text[MAX_TEXT];
}request_t;

static request_t queue[QUEUE_SIZE];
static int q_head = 0;
static int q_tail = 0;
static int q_count = 0;

static pthread_mutex_t q_mutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t q_not_empty = PTHREAD_COND_INITIALIZER;
static pthread_cond_t q_not_full = PTHREAD_COND_INITIALIZER;

static int next_request_id = 1;
static pthread_mutex_t id_mutex = PTHREAD_MUTEX_INITIALIZER;

static void enqueue(request_t request)
{
    pthread_mutex_lock(&q_mutex);

    while(q_count == QUEUE_SIZE)
    {
        printf("Queue is full (enqueue) - producer is waiting \n");
        pthread_cond_wait(&q_not_full, &q_mutex);
    }
    queue[q_tail] = request;
    q_tail = (q_tail + 1) % QUEUE_SIZE;
    q_count++;
    pthread_cond_signal(&q_not_empty);
    pthread_mutex_unlock(&q_mutex);
}

static request_t dequeue(void) {
    pthread_mutex_lock(&q_mutex);

    while(q_count == 0)
    {
        printf("Queue is empty (dequeue) - consumer is waiting \n");
        pthread_cond_wait(&q_not_empty, &q_mutex);
    }

    request_t request = queue[q_head];
    q_head = (q_head + 1) % QUEUE_SIZE;
    q_count--;

    pthread_cond_signal(&q_not_full);
    pthread_mutex_unlock(&q_mutex);

    return request;
}

static void* client_thread(void* arg)
{
    int client_id = *(int*)arg;
    free(arg);

    for(int iterator = 0; iterator < REQUESTS_PER_CLIENT; ++iterator)
    {
        pthread_mutex_lock(&id_mutex);
        int id = next_request_id++;
        pthread_mutex_unlock(&id_mutex);

        request_t req;
        req.id = id;
        snprintf(req.text, MAX_TEXT, "Client %d request #%d", client_id, iterator + 1);
        printf("[client %d] producing request id = %d \n", client_id, id);
        enqueue(req);

        usleep((rand() % 500 + 100) * 1000);

        printf("[client %d] finished producing.. \n", client_id);
    }

    return NULL;
}

//worker thread (consumer)
static void* worker_thread(void* arg)
{
    int worker_id = *(int*)arg;
    free(arg);

    while (1)
    {
        request_t req = dequeue();

        if(req.id == -1)
        {
            printf("[worker %d] received shutdown signal, exiting... \n", worker_id);
            break;
        }

        printf("[worker %d] processing request id = %d text = \"%s\" \n", worker_id, req.id, req.text);

        usleep((rand() % 500 + 200) * 1000);
        
        printf("[worker %d] done request id = %d \n", worker_id, req.id);
    }

    return NULL;
}

int main(void) {
    srand((unsigned)time(NULL));
    pthread_t clients[MAX_CLIENTS];
    pthread_t workers[WORKER_COUNT];

    for (int iterator = 0; iterator < WORKER_COUNT; ++iterator)
    {
        int* wid = malloc(sizeof(int));
        *wid = iterator + 1;
        pthread_create(&workers[iterator], NULL, worker_thread, wid);
    }
    for (int iterator = 0; iterator < MAX_CLIENTS; ++iterator)
    {
        int* cid = malloc(sizeof(int));
        *cid = iterator + 1;
        pthread_create(&clients[iterator], NULL, client_thread, cid);
    }
    for (int iterator = 0; iterator < MAX_CLIENTS; ++iterator)
    {
        pthread_join(clients[iterator], NULL);
    }
    printf(" sending shutdown signals to the workers \n");
    for (int iterator = 0; iterator < WORKER_COUNT; ++iterator)
    {
        request_t signal = { .id = -1, .text = ""};
        enqueue(signal);
    }
    for (int iterator = 0; iterator < WORKER_COUNT; ++iterator)
    {
        pthread_join(workers[iterator], NULL);
    }

    printf("[main] all workers exited - terminating the program \n");

    pthread_mutex_destroy(&q_mutex);
    pthread_mutex_destroy(&id_mutex);
    pthread_cond_destroy(&q_not_empty);
    pthread_cond_destroy(&q_not_full);

    return 0;
}