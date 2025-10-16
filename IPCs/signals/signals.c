#include <stdio.h>
#include <signal.h>
#include <unistd.h>

void handler(int signum){
    if(signum == SIGINT)
        printf("caught SIGINT!\n");
}

int main(){
    struct sigaction sa;
    sa.sa_handler = handler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;
    sigaction(SIGINT, &sa, NULL);
    printf("Press ctrl+c to send Interrupt!\n");
    while(1){
        printf("Process Running...\n");
        sleep(2);
    }
    return 0;
}