#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <sys/time.h>
#include <time.h>
#include <unistd.h>

#define INTERVAL_SEC 1

static struct timespec start_time;

void print_elapsed_time() {
    struct timespec now;
    clock_gettime(CLOCK_MONOTONIC, &now);

    double elapsed = (now.tv_sec - start_time.tv_sec) + (now.tv_nsec - start_time.tv_nsec) / 1e9;

    printf("Elapsed time: %.3f seconds\n", elapsed);
}

void run_task() {
    static int count = 0;
    count++;
    printf("Task #%d executed\n", count);
    print_elapsed_time();
}

void handle_sigalrm(int signum) {
    run_task();
}

void setup_timer(int interval_sec) {
    struct itimerval timer;
    timer.it_interval.tv_sec = interval_sec;
    timer.it_interval.tv_usec = 0;
    timer.it_value.tv_sec = interval_sec;
    timer.it_value.tv_usec = 0;

    if (setitimer(ITIMER_REAL, &timer, NULL) == -1) {
        perror("setitimer");
        exit(EXIT_FAILURE);
    }
}

// 🧵 Entry point
int main() {
    // Record start time
    clock_gettime(CLOCK_MONOTONIC, &start_time);
    struct sigaction sa;
    sa.sa_handler = handle_sigalrm;
    sa.sa_flags = SA_RESTART;
    sigemptyset(&sa.sa_mask);
    if (sigaction(SIGALRM, &sa, NULL) == -1) {
        perror("sigaction");
        exit(EXIT_FAILURE);
    }
    setup_timer(INTERVAL_SEC);
    while (1) {
        pause();
    }

    return 0;
}