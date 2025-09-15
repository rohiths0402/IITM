#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <stdatomic.h>
#include <time.h>

#define THREADS 32
#define INCREMENTS_PER_THREAD 100000

atomic_long atomic_counter = 0;
long mutex_counter = 0;
pthread_mutex_t counter_lock = PTHREAD_MUTEX_INITIALIZER;

typedef struct {
    int id;
    long iterations;
} thread_arg_t;

void* atomic_worker(void* arg) {
    thread_arg_t* t = (thread_arg_t*)arg;
    for (long i = 0; i < t->iterations; ++i) {
        atomic_fetch_add(&atomic_counter, 1);
    }
    return NULL;
}

void* mutex_worker(void* arg) {
    thread_arg_t* t = (thread_arg_t*)arg;
    for (long i = 0; i < t->iterations; ++i) {
        pthread_mutex_lock(&counter_lock);
        mutex_counter++;
        pthread_mutex_unlock(&counter_lock);
    }
    return NULL;
}

double benchmark(void* (*worker)(void*), const char* label) {
    pthread_t threads[THREADS];
    thread_arg_t args[THREADS];
    clock_t start = clock();

    for (int i = 0; i < THREADS; ++i) {
        args[i].id = i;
        args[i].iterations = INCREMENTS_PER_THREAD;
        pthread_create(&threads[i], NULL, worker, &args[i]);
    }

    for (int i = 0; i < THREADS; ++i) {
        pthread_join(threads[i], NULL);
    }

    clock_t end = clock();
    double elapsed = (double)(end - start) / CLOCKS_PER_SEC;
    printf("%s completed in %.6f seconds\n", label, elapsed);
    return elapsed;
}

int main() {
    printf("Benchmarking with %d threads, %d increments each...\n\n", THREADS, INCREMENTS_PER_THREAD);
    atomic_store(&atomic_counter, 0);
    double atomic_time = benchmark(atomic_worker, "Lock-Free Counter");
    printf("Final atomic counter: %ld\n\n", atomic_load(&atomic_counter));
    mutex_counter = 0;
    double mutex_time = benchmark(mutex_worker, "Mutex-Based Counter");
    printf("Final mutex counter: %ld\n\n", mutex_counter);
    printf("Speedup: %.2fx faster using atomics\n", mutex_time / atomic_time);
    return 0;
}