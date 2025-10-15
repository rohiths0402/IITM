#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>

int main(){
    printf("before execvp\n");
    char *args[] = {"ls", "-1", NULL};
    execvp("ls", args);
    perror("execvp fail");
    exit(1);
}