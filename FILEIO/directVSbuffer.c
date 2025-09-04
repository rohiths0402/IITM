#define _GNU_SOURCE
#define _FILE_OFFSET_BITS 64

#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<unistd.h>
#include<stdint.h>
#include<errno.h>
#include<time.h>
#include<sys/stat.h>
#include<sys/types.h>
#include<sys/statvfs.h>
#include<fcntl.h>

#ifndef __linux__
#define O_DIRECT 0 
#endif

static long long timespec_to_ns(const struct timespec *t)
{
    return (long long)t->tv_sec * 1000000000LL + t->tv_nsec;
}

static long long diff_ns(const struct timespec *a, const struct timespec *b)
{
    return timespec_to_ns(b) - timespec_to_ns(a);
}

static void fill_pattern(unsigned char *buffer, size_t length)
{
    for (size_t iterator = 0; iterator < length; ++iterator)
    {
        buffer[iterator] = (unsigned char)(iterator & 0xFF);
    }
}

static void benchmark_write(const char *label, int fd, unsigned char *buffer, size_t write_size, size_t iterations, int do_fsync)
{
    struct timespec t0, t1;

    long long min_ns = (1LL << 62), max_ns = 0, sum_ns = 0;

    off_t offset = 0;

    printf("WRITE : %s : write-size = %zu, iterations = %zu, do_fsync = %d \n", label, write_size, iterations, do_fsync);

    for (size_t iterator = 0; iterator < iterations; ++iterator)
    {
        if(clock_gettime(CLOCK_MONOTONIC, &t0) < 0)
        {
            perror("clock get time");
            exit(1);
        }

        size_t w = write(fd, buffer, write_size);

        if(w < 0)
        {
            perror("write");
            exit(1);
        }

        if((size_t)w != write_size)
        {
            fprintf(stderr, "short write %zd (expected %zu) \n", w, write_size);
            exit(1);
        }

        if(do_fsync)
        {
            if(fsync(fd) < 0)
            {
                perror("fsync");
                exit(1);
            }
        }

        if(clock_gettime(CLOCK_MONOTONIC, &t1) < 0)
        {
            perror("clock get time");
            exit(1);
        }

        long long ns = diff_ns(&t0, &t1);

        if(ns < min_ns)
        {
            min_ns = ns;
        }

        if(ns > max_ns)
        {
            max_ns = ns;
        }

        sum_ns += ns;
        offset += write_size;
    }

    double avg_ns = (double)sum_ns/iterations;
    double total_mb = (double)write_size * iterations / (1024.0 * 1024.0);
    double total_seconds = (double)total_mb / 1e9;

    printf("Results %s : ops = %zu, total_MB = %.2f, avg_op = %.2f ms, min = %.2f ms, max = %.2f ms, throughput = %.2f MB/s \n", label, iterations, total_mb, avg_ns/1e6, min_ns/1e6, max_ns/1e6, total_seconds > 0.0 ? (total_mb/total_seconds) : 0.0);
}

static void benchmark_read(const char *label, int fd, unsigned char *buffer, size_t read_size, size_t iterations)
{
    struct timespec t0, t1;

    long long min_ns = (1LL << 62), max_ns = 0, sum_ns = 0;

    printf("READ : %s : read-size = %zu, iterations = %zu \n", label, read_size, iterations);

    if(lseek(fd, 0, SEEK_SET) < 0)
    {
        perror("lseek");
        exit(1);
    }

    for(size_t iterator = 0; iterator < iterations; ++iterator)
    {
        if(clock_gettime(CLOCK_MONOTONIC, &t0) < 0)
        {
            perror("clock get time");
            exit(1);
        }

        size_t r = read(fd, buffer, read_size);
        if(r < 0)
        {
            perror("read");
            exit(1);
        }

        if((size_t)r != read_size)
        {
            fprintf(stderr, "short read %zd (expected %zu) \n", r, read_size);
            exit(1);
        }

        if(clock_gettime(CLOCK_MONOTONIC, &t1) < 0)
        {
            perror("clock get time");
            exit(1);
        }

        long long ns = diff_ns(&t0, &t1);

        if(ns < min_ns)
        {
            min_ns = ns;
        }

        if(ns > max_ns)
        {
            max_ns = ns;
        }

        sum_ns += ns;
    }

    double avg_ns = (double)sum_ns/iterations;
    double total_mb = (double)read_size * iterations / (1024.0 * 1024.0);
    double total_seconds = (double)total_mb / 1e9;

    printf("Results %s : ops = %zu, total_MB = %.2f, avg_op = %.2f ms, min = %.2f ms, max = %.2f ms, throughput = %.2f MB/s \n", label, iterations, total_mb, avg_ns/1e6, min_ns/1e6, max_ns/1e6, total_seconds > 0.0 ? (total_mb/total_seconds) : 0.0);
}

int main(int argc, char **argv)
{
    const char *filename = "/tmp/test_direct_io.bin";

    size_t write_size = 4096;
    size_t total_mb = 64;
    size_t align = 0;

    if(argc >= 2)
    {
        filename = argv[1];
    }

    if(argc >= 3)
    {
        write_size = (size_t)atoi(argv[2]);
    }

    if(argc >= 4)
    {
        write_size = (size_t)atoi(argv[3]);
    }

    size_t iterations = (total_mb * 1024 * 1024) / write_size;

    if(iterations == 0)
    {
        fprintf(stderr, "Either increase total_mb or reduce write_size \n");
        return 1;
    }

    printf("file = %s, write_size = %zu, total_mb = %zu iterations = %zu \n", filename, write_size, total_mb, iterations);

    // Normal (cached) read/write
    int fd_buffer = open(filename, O_CREAT | O_WRONLY | O_TRUNC, 0644);

    if(fd_buffer < 0)
    {
        perror("buffered open");
        return 1;
    }

    unsigned char *buffer = malloc(write_size);
    if(!buffer)
    {
        perror("malloc");
        return 1;
    }

    fill_pattern(buffer, write_size);

    benchmark_write("buffered", fd_buffer, buffer, write_size, iterations, 1);

    close(fd_buffer);

    //Direct IO read/write
    int fd_tmp = open(filename, O_CREAT | O_RDWR, 0644);

    if(fd_tmp < 0)
    {
        perror("open temp");
        return 1;
    }
    struct stat st;

    if(fstat(fd_tmp, &st) < 0)
    {
        perror("fstat");
        close(fd_tmp);
        return 1;
    }

    close(fd_tmp);

    long page_size = sysconf(_SC_PAGESIZE);
    align = (size_t)((st.st_blksize > 0 ? st.st_blksize : page_size));

    if(align < (size_t)page_size)
    {
        align = page_size;
    }

    #ifdef __linux__
    int fd_direct = open(filename, O_CREAT | O_WRONLY | O_TRUNC | O_DIRECT, 0644);
    #elif __APPLE__
        int fd_direct = open(filename, O_CREAT | O_WRONLY | O_TRUNC, 0644);
        if (fd_direct >= 0) {
            fcntl(fd_direct, F_NOCACHE, 1);  // macOS equivalent
        }
    #endif


    // int fd_direct = open(filename, O_CREAT | O_WRONLY | O_TRUNC | O_DIRECT, 0644);

    if(fd_direct < 0)
    {
        perror("open O_DIRECT");
        fprintf(stderr, "O_DIRECT open failed, missing alignment");
        return 1;
    }

    void *dbuffer = NULL;
    if(posix_memalign(&dbuffer, align, write_size) != 0)
    {
        perror("posix_memalign");
        return 1;
    }

    memset(dbuffer, 0, write_size);
    fill_pattern((unsigned char *)dbuffer, write_size);

    benchmark_write("direct", fd_direct, (unsigned char *)dbuffer, write_size, iterations, 1);

    close(fd_direct);

    int fd_rbuf = open(filename, O_RDONLY);

    if(fd_rbuf < 0)
    {
        perror("open for read");
        return 1;
    }
    unsigned char *read_buffer = malloc(write_size);

    if(!read_buffer)
    {
        perror("malloc");
        return 1;
    }

 #ifdef __linux__
    posix_fadvise(fd_rbuf, 0, 0, POSIX_FADV_DONTNEED);
#endif

    // posix_fadvise(fd_buffer, 0, 0, POSIX_FADV_DONTNEED);

    benchmark_read("buffered (cold)", fd_buffer, read_buffer, write_size, iterations);
    close(fd_buffer);

    int fd_rdirect = open(filename, O_RDONLY | O_DIRECT);
    if(fd_rdirect < 1)
    {
        perror("open O_DIRECT read");
        return 1;
    }
    void *rd_buffer = NULL;
    if(posix_memalign(&rd_buffer, align, write_size != 0))
    {
        perror("posix_memalign rdbuffer");
        return 1;
    }
    benchmark_read("direct read", fd_direct, (unsigned char *)rd_buffer, write_size, iterations);
    close(fd_direct);
    free(buffer);
    free(dbuffer);
    free(read_buffer);
    free(rd_buffer);
    printf("Done \n");
    return 0; 
}