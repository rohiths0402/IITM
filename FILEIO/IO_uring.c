#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <time.h>
#include <liburing.h>

#define DEFAULT_FILE "/tmp/simple_io_uring.bin"
#define DEFAULT_BLOCK_SIZE 4096
#define DEFAULT_TOTAL_MB 64
#define QUEUE_DEPTH 64

void FillBuffer(char *buffer, int size){
    for(int i = 0; i < size; i++){
        buffer[i] = i % 256;
    }
}

long long TimeDiffNS(struct timespec start, struct timespec end){
    return(end.tv_sec - start.tv_sec) * 1000000000LL +(end.tv_nsec - start.tv_nsec);
}

int SetupUring(struct io_uring *ring){
    return io_uring_queue_init(QUEUE_DEPTH, ring, 0);
}

long long perform(struct io_uring *ring, int fd, char *buffer, int size, off_t offset){
    struct io_uring_sqe *sqe = io_uring_get_sqe(ring);
    io_uring_prep_write(sqe, fd, buffer, size, offset);
    struct timespec start, end;
    clock_gettime(CLOCK_MONOTONIC, &start);
    io_uring_submit(ring);
    struct io_uring_cqe *cqe;
    io_uring_wait_cqe(ring, &cqe);
    clock_gettime(CLOCK_MONOTONIC, &end);
    if(cqe->res < 0){
        fprintf(stderr, "Write failed: %d\n", cqe->res);
        io_uring_cqe_seen(ring, cqe);
        return -1;
    }
    io_uring_cqe_seen(ring, cqe);
    return TimeDiffNS(start, end);
}

void run_write_benchmark(const char *filename, int block_size, int total_mb){
    int total_bytes = total_mb * 1024 * 1024;
    int iterations = total_bytes / block_size;
    int fd = open(filename, O_CREAT | O_WRONLY | O_TRUNC, 0644);
    if(fd < 0){
        perror("open");
        return;
    }
    char *buffer = malloc(block_size);
    if(!buffer){
        perror("malloc");
        close(fd);
        return;
    }
    FillBuffer(buffer, block_size);
    struct io_uring ring;
    if(SetupUring(&ring) < 0){
        perror("io_uring_queue_init");
        free(buffer);
        close(fd);
        return;
    }
    long long total_ns = 0;
    for(int i = 0; i < iterations; i++){
        off_t offset = i * block_size;
        long long ns = perform(&ring, fd, buffer, block_size, offset);
        if(ns < 0) break;
        total_ns += ns;
    }
    double avg_ms =(double)total_ns / iterations / 1e6;
    printf("Write Benchmark: %d ops, avg time = %.3f ms\n", iterations, avg_ms);
    io_uring_queue_exit(&ring);
    close(fd);
    free(buffer);
}

void show_menu(){
    printf("\n=== IO_URING MENU ===\n");
    printf("1. Run Write Benchmark\n");
    printf("2. Exit\n");
    printf("Choose an option: ");
}

int main(){
    const char *filename = DEFAULT_FILE;
    int block_size = DEFAULT_BLOCK_SIZE;
    int total_mb = DEFAULT_TOTAL_MB;

    int choice;
    do{
        show_menu();
        scanf("%d", &choice);

        switch(choice){
            case 1:
                run_write_benchmark(filename, block_size, total_mb);
                break;
            case 2:
                printf("Exiting...\n");
                break;
            default:
                printf("Invalid option. Try again.\n");
        }
    } while(choice != 2);

    return 0;
}