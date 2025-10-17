#include <stdio.h>
#include <time.h>
#include <unistd.h>  

int main() {
    struct timespec start, end;

    clock_gettime(CLOCK_MONOTONIC, &start);
    sleep(2);
    clock_gettime(CLOCK_MONOTONIC, &end);
    double elapsed = (end.tv_sec - start.tv_sec) + (end.tv_nsec - start.tv_nsec) / 1e9;
    printf("Elapsed time: %.6f seconds\n", elapsed);
    return 0;
}
