#include <stdio.h>
#include <stdlib.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <sys/wait.h>
#include <unistd.h>

#define MAX_WORKERS 10

union semun{
    int val;
    struct semid_ds *buf;
    unsigned short *array;
};

void lock(int semid){
    struct sembuf op ={0, -1, 0};
    semop(semid, &op, 1);
}

void unlock(int semid){
    struct sembuf op ={0, 1, 0};
    semop(semid, &op, 1);
}

int main(){
    int num_workers, increments;
    printf("Enter number of workers(1–%d): ", MAX_WORKERS);
    scanf("%d", &num_workers);
    if(num_workers <= 0 || num_workers > MAX_WORKERS){
        fprintf(stderr, "Invalid number of workers.\n");
        exit(EXIT_FAILURE);
    }
    printf("Enter number of increments per worker: ");
    scanf("%d", &increments);
    if(increments <= 0){
        fprintf(stderr, "Invalid number of increments.\n");
        exit(EXIT_FAILURE);
    }
    key_t shm_key = ftok(".", 'M');
    key_t sem_key = ftok(".", 'S');
    int shm_id = shmget(shm_key, sizeof(int), IPC_CREAT | 0666);
    int *shared_total =(int *)shmat(shm_id, NULL, 0);
    *shared_total = 0;
    int sem_id = semget(sem_key, 1, IPC_CREAT | 0666);
    union semun sem_arg;
    sem_arg.val = 1;
    semctl(sem_id, 0, SETVAL, sem_arg);
    for(int i = 0; i < num_workers; i++){
        pid_t pid = fork();
        if(pid == 0){
            for(int j = 0; j < increments; j++){
                lock(sem_id);
               (*shared_total)++;
                unlock(sem_id);
            }
            shmdt(shared_total);
            exit(0);
        }
    }
    for(int i = 0; i < num_workers; i++){
        wait(NULL);
    }
    printf("Final shared total: %d\n", *shared_total);
    shmdt(shared_total);
    shmctl(shm_id, IPC_RMID, NULL);
    semctl(sem_id, 0, IPC_RMID);

    return 0;
}