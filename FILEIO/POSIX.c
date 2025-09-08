#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <aio.h>
#include <time.h>
#include <sys/types.h>
#include <sys/stat.h>

static long long timespec_to_ns(const struct timespec *t){
    return (long long)t->tv_sec * 1000000000LL + t->tv_nsec;
}

int main(int argc, char *argv[]) {
    const char *filename = "/tmp/aio_demo.bin";
    int total_mb = 2;
    size_t write_size = 4096;
    if(argc >= 2){
        total_mb = atoi(argv[1]);
    }
    if(argc >= 3){ 
        filename = argv[2];
    } 
    if(argc >= 4){
        write_size = (size_t)atoi(argv[3]);
    }
    size_t total_bytes = total_mb * 1024 * 1024;
    size_t iterations = total_bytes / write_size;
    if(iterations == 0){
        fprintf(stderr, "Either increase the total MB or reduce the write Size\n");
        return 1;
    }
    int fd = open(filename, O_CREAT | O_WRONLY | O_TRUNC, 0644);
    if(fd < 0){
        perror("open");
        return 1;
    }
    unsigned char *buffer = malloc(write_size);
    if(!buffer) {
        perror("malloc");
        close(fd);
        return 1;
    }
    for(size_t i = 0; i < write_size; ++i){
        buffer[i] = (unsigned char)(i & 0xFF);
    }
    struct aiocb *cbs = calloc(iterations, sizeof(struct aiocb));
    if(!cbs){
        perror("calloc");
        free(buffer);
        close(fd);
        return 1;
    }
    struct timespec tstart, tend;
    clock_gettime(CLOCK_MONOTONIC, &tstart);
    printf("POSIX AIO demo - %zu ops of %zu bytes\n", iterations, write_size);
    for(size_t i = 0; i < iterations; ++i){
        off_t offset = (off_t)write_size * i;
        memset(&cbs[i], 0, sizeof(struct aiocb));
        cbs[i].aio_fildes = fd;
        cbs[i].aio_buf = buffer;
        cbs[i].aio_nbytes = write_size;
        cbs[i].aio_offset = offset;
        if(aio_write(&cbs[i]) < 0){
            perror("aio_write");
            free(cbs);
            free(buffer);
            close(fd);
            return 1;
        }
    }
    for (size_t i = 0; i < iterations; ++i) {
        while (aio_error(&cbs[i]) == EINPROGRESS) {
           
        }
        int ret = aio_return(&cbs[i]);
        if(ret < 0){
            fprintf(stderr, "Write %zu failed: %s\n", i, strerror(errno));
        }
    }
    clock_gettime(CLOCK_MONOTONIC, &tend);
    long long elapsed_ns = timespec_to_ns(&tend) - timespec_to_ns(&tstart);
    double elapsed_sec = elapsed_ns / 1e9;
    double throughput_mb = total_mb / elapsed_sec;
    printf("Completed %zu writes in %.3f sec (%.2f MB/s)\n", iterations, elapsed_sec, throughput_mb);
    free(cbs);
    free(buffer);
    close(fd);
    return 0;
}