#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <sys/wait.h>

#define MAX_CHILDREN 3

pid_t children[MAX_CHILDREN];
int running = 1;
void spawn_child(int i){
    pid_t pid = fork();
    if(pid == 0){
        printf("Child %d started with PID %d\n", i, getpid());
        while(1){
            printf("Child %d working...\n", i);
            sleep(2);
        }
        exit(0);
    }
    else if(pid > 0){
        children[i] = pid;
    }
    else{
        perror("fork failed");
        exit(EXIT_FAILURE);
    }
}

void kill_children(){
    for(int i = 0; i < MAX_CHILDREN; i++){
        if(children[i] > 0){
            printf("Killing child PID %d\n", children[i]);
            kill(children[i], SIGTERM);
        }
    }
}

void restart_children(){
    printf("Restarting all children...\n");
    kill_children();
    sleep(1);
    for(int i = 0; i < MAX_CHILDREN; i++){
        spawn_child(i);
    }
}

void handle_signal(int sig){
    if(sig == SIGINT){
        printf("\nReceived SIGINT, stopping all children...\n");
        running = 0;
        kill_children();
    }
    else if(sig == SIGHUP){
        printf("\nReceived SIGHUP, restarting children...\n");
        restart_children();
    }
    else if(sig == SIGTERM){
        printf("\nReceived SIGTERM, terminating gracefully...\n");
        running = 0;
        kill_children();
    }
}

void handle_sigchld(int sig){
   (void)sig;
    int status;
    pid_t pid;
    while((pid = waitpid(-1, &status, WNOHANG)) > 0){
        printf("Reaped child process PID %d\n", pid);
    }
}

int main(){
    struct sigaction sa;
    sa.sa_handler = handle_signal;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;
    sigaction(SIGINT, &sa, NULL);
    sigaction(SIGHUP, &sa, NULL);
    sigaction(SIGTERM, &sa, NULL);
    struct sigaction sa_chld;
    sa_chld.sa_handler = handle_sigchld;
    sigemptyset(&sa_chld.sa_mask);
    sa_chld.sa_flags = SA_RESTART;
    sigaction(SIGCHLD, &sa_chld, NULL);
    for(int i = 0; i < MAX_CHILDREN; i++){
        spawn_child(i);
    }
    printf("Job controller running PID: %d\n", getpid());
    printf("Send SIGHUP to restart, SIGINT/SIGTERM to stop\n");
    while(running){
        pause();
    }
    printf("Job controller exiting cleanly\n");
    return 0;
}
