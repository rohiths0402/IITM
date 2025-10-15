#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <string.h>
#define MAX_INPUT 1024
#define MAX_ARGS 64

int main(){
    char input[MAX_INPUT];
    char *args[MAX_ARGS];
    while(1){
        printf("shell>");
        fflush(stdout);
        if(fgets(input, sizeof(input), stdin) == NULL){
            printf("\n");
            break;
        }
        input[strcspn(input, "\n")] =0;
        if(strcmp(input, "exit") == 0){
            break;
        }
        int i =0;
        char *token = strtok(input," \t");
        while(token!= NULL && i<MAX_ARGS -1)
        {
            args[i++] = token;
            token = strtok(NULL, " \t");
        }
        args[i] =NULL;
        if(args[0] == NULL){
            continue;
        }
        pid_t pid = fork();
        if(pid == 0){
            execvp(args[0], args);
            perror("execvp failed");
            exit(EXIT_FAILURE);
        }
        else if(pid >0){
            int status;
            waitpid(pid, &status,0);
        }
        else{
            perror("fork failed");
        }
    }
    return 0;
}