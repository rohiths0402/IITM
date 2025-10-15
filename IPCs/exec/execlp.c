#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>

int main(){
    printf("before execlp\n");
    execlp("ls", "ls", "-1", NULL);
    perror("execlp fail");
    exit(1);
}