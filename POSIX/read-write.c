#include <stdio.h>
#include <pthread.h>
#include <unistd.h>

int shared_data = 0;

typedef struct {
    pthread_mutex_t lock;
    pthread_cond_t readers_ok;
    pthread_cond_t writers_ok;
    int readers;
    int writers;
    int waiting_writers;
} rwlock_t;

void rwlock_init(rwlock_t* rw) {
    pthread_mutex_init(&rw->lock, NULL);
    pthread_cond_init(&rw->readers_ok, NULL);
    pthread_cond_init(&rw->writers_ok, NULL);
    rw->readers = 0;
    rw->writers = 0;
    rw->waiting_writers = 0;
}

void rwlock_rdlock(rwlock_t* rw) {
    pthread_mutex_lock(&rw->lock);
    while (rw->writers > 0 || rw->waiting_writers > 0) {
        pthread_cond_wait(&rw->readers_ok, &rw->lock);
    }
    rw->readers++;
    pthread_mutex_unlock(&rw->lock);
}

void rwlock_rdunlock(rwlock_t* rw) {
    pthread_mutex_lock(&rw->lock);
    rw->readers--;
    if (rw->readers == 0) {
        pthread_cond_signal(&rw->writers_ok);
    }
    pthread_mutex_unlock(&rw->lock);
}

void rwlock_wrlock(rwlock_t* rw) {
    pthread_mutex_lock(&rw->lock);
    rw->waiting_writers++;
    while (rw->writers > 0 || rw->readers > 0) {
        pthread_cond_wait(&rw->writers_ok, &rw->lock);
    }
    rw->waiting_writers--;
    rw->writers = 1;
    pthread_mutex_unlock(&rw->lock);
}

void rwlock_wrunlock(rwlock_t* rw) {
    pthread_mutex_lock(&rw->lock);
    rw->writers = 0;
    if (rw->waiting_writers > 0) {
        pthread_cond_signal(&rw->writers_ok);
    } else {
        pthread_cond_broadcast(&rw->readers_ok);
    }
    pthread_mutex_unlock(&rw->lock);
}

void rwlock_destroy(rwlock_t* rw) {
    pthread_mutex_destroy(&rw->lock);
    pthread_cond_destroy(&rw->readers_ok);
    pthread_cond_destroy(&rw->writers_ok);
}

rwlock_t rwlock;

void* reader(void* arg) {
    int id = *(int*)arg;
    while (1) {
        rwlock_rdlock(&rwlock);
        printf("Reader %d: read value %d\n", id, shared_data);
        rwlock_rdunlock(&rwlock);
        sleep(1);
    }
    return NULL;
}

void* writer(void* arg) {
    int id = *(int*)arg;
    while (1) {
        rwlock_wrlock(&rwlock);
        shared_data++;
        printf("Writer %d: wrote value %d\n", id, shared_data);
        rwlock_wrunlock(&rwlock);
        sleep(2);
    }
    return NULL;
}

int main() {
    pthread_t r1, r2, w1;
    int r1_id = 1, r2_id = 2, w1_id = 1;
    rwlock_init(&rwlock);
    pthread_create(&r1, NULL, reader, &r1_id);
    pthread_create(&r2, NULL, reader, &r2_id);
    pthread_create(&w1, NULL, writer, &w1_id);
    pthread_join(r1, NULL);
    pthread_join(r2, NULL);
    pthread_join(w1, NULL);
    rwlock_destroy(&rwlock);
    return 0;
}
