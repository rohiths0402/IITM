#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int main(){
    printf("before execl\n");
    execl("/bin/ls", "ls", "-1", NULL);
    perror("exec fail");
    exit(1);
}