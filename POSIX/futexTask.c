#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>

#define BUFFER_CAPACITY 10
#define ITEMS_PER_PRODUCER 10
#define TOTAL_PRODUCERS 5
#define TOTAL_CONSUMERS 5

int buffer[BUFFER_CAPACITY];
int head = 0, tail = 0, item_count = 0;

pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t buffer_not_full = PTHREAD_COND_INITIALIZER;
pthread_cond_t buffer_not_empty = PTHREAD_COND_INITIALIZER;

void push_to_buffer(int item) {
    pthread_mutex_lock(&lock);

    while (item_count == BUFFER_CAPACITY) {
        pthread_cond_wait(&buffer_not_full, &lock);
    }

    buffer[tail] = item;
    tail = (tail + 1) % BUFFER_CAPACITY;
    item_count++;

    pthread_cond_signal(&buffer_not_empty);
    pthread_mutex_unlock(&lock);
}

int pop_from_buffer() {
    pthread_mutex_lock(&lock);

    while (item_count == 0) {
        pthread_cond_wait(&buffer_not_empty, &lock);
    }

    int item = buffer[head];
    head = (head + 1) % BUFFER_CAPACITY;
    item_count--;

    pthread_cond_signal(&buffer_not_full);
    pthread_mutex_unlock(&lock);

    return item;
}

void* producer_thread(void* arg) {
    int producer_id = *((int*)arg);
    for (int i = 0; i < ITEMS_PER_PRODUCER; ++i) {
        int item = producer_id * 100 + i;
        push_to_buffer(item);
        printf("Producer %d added item %d\n", producer_id, item);
        usleep(rand() % 150000);
    }
    return NULL;
}

void* consumer_thread(void* arg) {
    int consumer_id = *((int*)arg);
    int items_to_consume = (TOTAL_PRODUCERS * ITEMS_PER_PRODUCER) / TOTAL_CONSUMERS;

    for (int i = 0; i < items_to_consume; ++i) {
        int item = pop_from_buffer();
        printf("Consumer %d processed item %d\n", consumer_id, item);
        usleep(rand() % 250000);
    }
    return NULL;
}

int main() {
    pthread_t producer_threads[TOTAL_PRODUCERS];
    pthread_t consumer_threads[TOTAL_CONSUMERS];
    int producer_ids[TOTAL_PRODUCERS];
    int consumer_ids[TOTAL_CONSUMERS];

    for (int i = 0; i < TOTAL_PRODUCERS; ++i) {
        producer_ids[i] = i + 1;
        if (pthread_create(&producer_threads[i], NULL, producer_thread, &producer_ids[i]) != 0) {
            perror("Failed to create producer thread");
            exit(EXIT_FAILURE);
        }
    }

    for (int i = 0; i < TOTAL_CONSUMERS; ++i) {
        consumer_ids[i] = i + 1;
        if (pthread_create(&consumer_threads[i], NULL, consumer_thread, &consumer_ids[i]) != 0) {
            perror("Failed to create consumer thread");
            exit(EXIT_FAILURE);
        }
    }

    for (int i = 0; i < TOTAL_PRODUCERS; ++i) {
        pthread_join(producer_threads[i], NULL);
    }

    for (int i = 0; i < TOTAL_CONSUMERS; ++i) {
        pthread_join(consumer_threads[i], NULL);
    }

    return 0;
}
