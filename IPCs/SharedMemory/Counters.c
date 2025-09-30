#include <stdio.h>
#include <stdlib.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/wait.h>
#include <unistd.h>

#define NUM_WORKERS 4
#define INCREMENTS_PER_WORKER 100000

int main(){
    key_t key = ftok("sysv_shared_counter.c", 'A');
    if(key == -1){
        perror("ftok");
        exit(EXIT_FAILURE);
    }
    int shm_id = shmget(key, sizeof(int) * NUM_WORKERS, IPC_CREAT | 0666);
    if(shm_id == -1){
        perror("shmget");
        exit(EXIT_FAILURE);
    }
    int *counters =(int *)shmat(shm_id, NULL, 0);
    if(counters ==(void *)-1){
        perror("shmat");
        exit(EXIT_FAILURE);
    }
    for(int i = 0; i < NUM_WORKERS; i++){
        counters[i] = 0;
    }
    for(int i = 0; i < NUM_WORKERS; i++){
        int worker_id = i; 
        pid_t pid = fork();
        if(pid == 0){
            for(int j = 0; j < INCREMENTS_PER_WORKER; j++){
                counters[worker_id]++;
            }
            shmdt(counters);
            exit(0);
        } else if(pid < 0){
            perror("fork");
            exit(EXIT_FAILURE);
        }
    }

    for(int i = 0; i < NUM_WORKERS; i++){
        wait(NULL);
    }
    int total = 0;
    for(int i = 0; i < NUM_WORKERS; i++){
        printf("Worker %d counter: %d\n", i, counters[i]);
        total += counters[i];
    }
    printf("Total count: %d\n", total);

    shmdt(counters);
    shmctl(shm_id, IPC_RMID, NULL);

    return 0;
}