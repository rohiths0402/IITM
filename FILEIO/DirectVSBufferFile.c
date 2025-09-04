#define _GNU_SOURCE
#define _FILE_OFFSET_BITS 64

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdint.h>
#include <errno.h>
#include <time.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/statvfs.h>
#include <getopt.h>

#ifndef __linux__
#define O_DIRECT 0
#endif

static inline long long ns_from_timespec(const struct timespec *t){
    return(long long)t->tv_sec * 1000000000LL + t->tv_nsec;
}

static inline long long ns_diff(const struct timespec *a, const struct timespec *b){
    return ns_from_timespec(b) - ns_from_timespec(a);
}

static void fill_buffer(unsigned char *buf, size_t len){
    for(size_t i = 0; i < len; ++i)
        buf[i] =(unsigned char)(i & 0xFF);
}

static void run_write_benchmark(const char *label, int fd, unsigned char *buf, size_t size, size_t count, int sync){
    struct timespec t0, t1;
    long long min = INT64_MAX, max = 0, total = 0;
    printf("WRITE [%s] size=%zu, count=%zu, fsync=%d\n", label, size, count, sync);
    for(size_t i = 0; i < count; ++i){
        if(clock_gettime(CLOCK_MONOTONIC, &t0) < 0){ 
            perror("clock"); exit(1); 
        }
        ssize_t written = write(fd, buf, size);
        if(written < 0 ||(size_t)written != size){
            fprintf(stderr, "Write failed: %zd(expected %zu)\n", written, size);
            exit(1);
        }
        if(sync && fsync(fd) < 0){ 
            perror("fsync"); exit(1); 
        }
        if(clock_gettime(CLOCK_MONOTONIC, &t1) < 0){ 
            perror("clock"); exit(1); 
        }
        long long ns = ns_diff(&t0, &t1);
        if(ns < min) min = ns;
        if(ns > max) max = ns;
        total += ns;
    }

    double avg =(double)total / count;
    double mb =(double)(size * count) /(1024.0 * 1024.0);
    double sec =(double)total / 1e9;
    double throughput = sec > 0.0 ? mb / sec : 0.0;

    printf("RESULT [%s] ops=%zu, MB=%.2f, avg=%.2fms, min=%.2fms, max=%.2fms, throughput=%.2fMB/s\n",
           label, count, mb, avg / 1e6, min / 1e6, max / 1e6, throughput);
}

static void run_read_benchmark(const char *label, int fd, unsigned char *buf, size_t size, size_t count){
    struct timespec t0, t1;
    long long min = INT64_MAX, max = 0, total = 0;

    printf("READ [%s] size=%zu, count=%zu\n", label, size, count);

    if(lseek(fd, 0, SEEK_SET) < 0){
        perror("lseek"); exit(1);
    }
    for(size_t i = 0; i < count; ++i){
        if(clock_gettime(CLOCK_MONOTONIC, &t0) < 0){ perror("clock"); exit(1); }

        ssize_t read_bytes = read(fd, buf, size);
        if(read_bytes < 0 ||(size_t)read_bytes != size){
            fprintf(stderr, "Read failed: %zd(expected %zu)\n", read_bytes, size);
            exit(1);
        }
        if(clock_gettime(CLOCK_MONOTONIC, &t1) < 0)
      { 
            perror("clock"); exit(1); 
        }
        long long ns = ns_diff(&t0, &t1);
        if(ns < min) min = ns;
        if(ns > max) max = ns;
        total += ns;
    }
    double avg =(double)total / count;
    double mb =(double)(size * count) /(1024.0 * 1024.0);
    double sec =(double)total / 1e9;
    double throughput = sec > 0.0 ? mb / sec : 0.0;
    printf("RESULT [%s] ops=%zu, MB=%.2f, avg=%.2fms, min=%.2fms, max=%.2fms, throughput=%.2fMB/s\n",
           label, count, mb, avg / 1e6, min / 1e6, max / 1e6, throughput);
}

int main(int argc, char **argv){
    const char *file = "/destination.txt";
    size_t block_size = 4096;
    size_t total_mb = 64;
    int run_buffered = 1, run_direct = 1;
    static struct option long_opts[] ={
       {"file",  required_argument, NULL, 1},
        {"block", required_argument, NULL, 2},
        {"size",  required_argument, NULL, 3},
        {"mode",  required_argument, NULL, 4},
        {"exit",  no_argument,       NULL, 5},
        {0, 0, 0, 0}

    };
    int opt ,option_index = 0;
    do {
        printf("\nMenu:\n");
        printf("1. FILE\n2. BLOCK\n3. SIZE\n4. MODE\n5. EXIT \nEnter Choice: ");
        if (scanf("%d", &opt) != 1) {
            fprintf(stderr, "Invalid input. Exiting.\n");
            break;
        }
        switch (opt){
            case 1: 
                file = optarg;
                break;
            case 2:
                block_size = (size_t)atoi(optarg);
                break;
            case 3: 
                total_mb = (size_t)atoi(optarg);
                break;
            case 4: 
                if(strcmp(optarg, "buffered") == 0){
                    run_buffered = 1; run_direct = 0;
                } 
                else if (strcmp(optarg, "direct") == 0){
                    run_buffered = 0; run_direct = 1;
                } 
                else if(strcmp(optarg, "both") == 0){
                    run_buffered = 1; run_direct = 1;
                } 
                else{
                    fprintf(stderr, "Invalid mode: %s\n", optarg);
                    return 1;
                }
                break;
            case 5:
                printf("Exit requested. Terminating early.\n");
                return 0;
            case -1:
                break;
            default:
                fprintf(stderr, "Usage: --file <path> --block <size> --size <MB> --mode <buffered|direct|both> [--exit]\n");
                return 1;
        }
    } while (opt != -1);
    size_t iterations =(total_mb * 1024 * 1024) / block_size;
    if(iterations == 0){
        fprintf(stderr, "Invalid config: iterations = 0\n");
        return 1;
    }
    printf("Config: file=%s, block_size=%zu, total_mb=%zu, iterations=%zu\n",file, block_size, total_mb, iterations);
    long page_size = sysconf(_SC_PAGESIZE);
    size_t align =(size_t)page_size;
    if(run_buffered){
        int fd_buf = open(file, O_CREAT | O_WRONLY | O_TRUNC, 0644);
        if(fd_buf < 0){ perror("open buffered"); return 1; }
        unsigned char *buf = malloc(block_size);
        if(!buf){ perror("malloc"); return 1; }
        fill_buffer(buf, block_size);
        run_write_benchmark("buffered", fd_buf, buf, block_size, iterations, 1);
        close(fd_buf);
        int fd_rbuf = open(file, O_RDONLY);
        if(fd_rbuf < 0){ perror("open read buffered"); return 1; }

    #ifdef __linux__
        posix_fadvise(fd_rbuf, 0, 0, POSIX_FADV_DONTNEED);
    #endif

        unsigned char *rbuf = malloc(block_size);
        if(!rbuf){ perror("malloc read"); return 1; }
        run_read_benchmark("buffered(cold)", fd_rbuf, rbuf, block_size, iterations);
        close(fd_rbuf);
        free(buf);
        free(rbuf);
    }
    if(run_direct){
        void *dbuf = NULL;
        if(posix_memalign(&dbuf, align, block_size) != 0){
            perror("posix_memalign"); return 1;
        }
        fill_buffer((unsigned char *)dbuf, block_size);
        int fd_direct = open(file, O_CREAT | O_WRONLY | O_TRUNC | O_DIRECT, 0644);
        if(fd_direct < 0){ perror("open direct"); return 1; }
        run_write_benchmark("direct", fd_direct,(unsigned char *)dbuf, block_size, iterations, 1);
        close(fd_direct);
        void *rdirect_buf = NULL;
        if(posix_memalign(&rdirect_buf, align, block_size) != 0){
            perror("posix_memalign read"); return 1;
        }
        int fd_rdirect = open(file, O_RDONLY | O_DIRECT);
        if(fd_rdirect < 0){ perror("open direct read"); return 1; }
        run_read_benchmark("direct read", fd_rdirect,(unsigned char *)rdirect_buf, block_size, iterations);
        close(fd_rdirect);
        free(dbuf);
        free(rdirect_buf);
    }
    printf("Benchmark complete.\n");
    return 0;
}