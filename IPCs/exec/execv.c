#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>

int main(){
    printf("before execv\n");
    char *args[] = {"ls", "-1", NULL};
    execv("/bin/ls", args);
    perror("execv fail");
    exit(1);
}