#define _GNU_SOURCE
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>

int main(){
    printf("before execvpe\n");
    char *args[] = {"ls", "-1", NULL};
    char *envp[] = {"MYVAR=HELLOWORLD", NULL};
    execvpe("ls", args, envp);
    perror("execv fail");
    exit(1);
}