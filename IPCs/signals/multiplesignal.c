#include <stdio.h>
#include <signal.h>
#include <unistd.h>

void handler(int signum){
    if(signum == SIGINT)
        printf("Caught SIGINT!\n");
    else if(signum == SIGTERM)
        printf("Caught SIGTERM!\n");
    else if(signum == SIGUSR1)
        printf("Caught SIGUSR1!\n");
}

int main(){
    struct sigaction sa;
    sa.sa_handler = handler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;
    
    sigaction(SIGINT, &sa, NULL);
    sigaction(SIGTERM, &sa, NULL);
    sigaction(SIGUSR1, &sa, NULL);

    printf("[PID: %d] -> Try \'kill\' to Terminate the process \
        or try \'kill -SIGTERM\' or \'kill -SIGUSR1\'\n", getpid());
    
    while(1){
        printf("Process Running...\n");
        sleep(1);
    }
    return 0;
}