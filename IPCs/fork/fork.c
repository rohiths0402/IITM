#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>

int main()
{
    printf("parent process id: %d\n",getpid());

    pid_t pid=fork();
    
    if(pid<0)
    {
        perror("fork failed");
        exit(1);
    }
    else if(pid==0)
    {
        printf("Hello from child process\n");
        printf("child process id: %d\n",getpid());
    }
    else
    {
        printf("Hello from parent\n");
        printf("parent process id: %d\n",getpid());
        printf("child process id: %d\n",pid);
    }
}