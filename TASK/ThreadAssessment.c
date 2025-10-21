#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>

#define NUM_WORKERS 3
#define NUM_TASKS 10

typedef struct {
    int tasks[NUM_TASKS];
    int front;
    int rear;
    int count;
    pthread_mutex_t lock;
    pthread_cond_t cond;
} TaskQueue;

TaskQueue queue;
void* worker_thread(void* arg) {
    int id = *(int*)arg;
    free(arg);

    while (1) {
        pthread_mutex_lock(&queue.lock);
        while (queue.count == 0) {
            pthread_cond_wait(&queue.cond, &queue.lock);
        }
        int num = queue.tasks[queue.front];
        queue.front = (queue.front + 1) % NUM_TASKS;
        queue.count--;
        pthread_mutex_unlock(&queue.lock);
        printf("Worker %d processing number %d -> square = %d\n", id, num, num * num);
        usleep(100000);
        if (queue.count == 0)
            break;
    }

    return NULL;
}

int main() {
    pthread_t workers[NUM_WORKERS];
    queue.front = 0;
    queue.rear = 0;
    queue.count = 0;
    pthread_mutex_init(&queue.lock, NULL);
    pthread_cond_init(&queue.cond, NULL);
    for (int i = 0; i < NUM_WORKERS; i++) {
        int* id = malloc(sizeof(int));
        *id = i + 1;
        pthread_create(&workers[i], NULL, worker_thread, id);
    }
    pthread_mutex_lock(&queue.lock);
    for (int i = 0; i < NUM_TASKS; i++) {
        queue.tasks[queue.rear] = i + 1;
        queue.rear = (queue.rear + 1) % NUM_TASKS;
        queue.count++;
    }
    pthread_cond_broadcast(&queue.cond);
    pthread_mutex_unlock(&queue.lock);
    for (int i = 0; i < NUM_WORKERS; i++) {
        pthread_join(workers[i], NULL);
    }
    pthread_mutex_destroy(&queue.lock);
    pthread_cond_destroy(&queue.cond);
    printf("All tasks processed. Exiting.\n");
    return 0;
}
