#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>

int main(){
    printf("before execle\n");
    char *envp[] = {"MYVAR=HELLOWORLD", NULL};
    execle("/bin/ls", "ls", "-1", NULL, envp);
    perror("execle fail");
    exit(1);
}