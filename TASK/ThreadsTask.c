#include <stdio.h>
#include <pthread.h>
#include <unistd.h>

#define MAX 20

pthread_mutex_t lock;  
pthread_cond_t cond;
int count = 1;

void* print_odd(void* arg) {
    while (count <= MAX) {
        pthread_mutex_lock(&lock);
        while (count % 2 == 0)
            pthread_cond_wait(&cond, &lock);
        if (count <= MAX) {
            printf("Odd Thread: %d\n", count);
            count++;
            pthread_cond_signal(&cond);
        }
        pthread_mutex_unlock(&lock);
    }
    return NULL;
}

void* print_even(void* arg) {
    while (count <= MAX) {
        pthread_mutex_lock(&lock);
        while (count % 2 != 0)
            pthread_cond_wait(&cond, &lock);
        if (count <= MAX) {
            printf("Even Thread: %d\n", count);
            count++;
            pthread_cond_signal(&cond);
        }
        pthread_mutex_unlock(&lock);
    }
    return NULL;
}

int main() {
    pthread_t t1, t2;
    pthread_mutex_init(&lock, NULL);
    pthread_cond_init(&cond, NULL);
    pthread_create(&t1, NULL, print_odd, NULL);
    pthread_create(&t2, NULL, print_even, NULL);
    pthread_join(t1, NULL);
    pthread_join(t2, NULL);
    pthread_mutex_destroy(&lock);
    pthread_cond_destroy(&cond);

    return 0;
}
