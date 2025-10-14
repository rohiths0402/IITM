#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <pthread.h>
#include <arpa/inet.h>
#include <sys/time.h>

#define PORT 8080
#define BACKLOG 10
#define BUF_SIZE 1024

int mode = 0; 
void handle_client(int client_fd) {
    char buffer[BUF_SIZE];
    read(client_fd, buffer, BUF_SIZE);
    usleep(1000);
    char response[] = "OK\n";
    write(client_fd, response, strlen(response));
    close(client_fd);
}

void *thread_worker(void *arg) {
    int client_fd = *(int *)arg;
    free(arg);
    handle_client(client_fd);
    return NULL;
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        printf("Usage: %s [fork|thread]\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    if (strcmp(argv[1], "thread") == 0)
        mode = 1;

    int server_fd, client_fd;
    struct sockaddr_in addr;
    socklen_t addrlen = sizeof(addr);

    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd == -1) {
        perror("socket");
        exit(EXIT_FAILURE);
    }

    int opt = 1;
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
        perror("setsockopt");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(PORT);

    if (bind(server_fd, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
        perror("bind");
        exit(EXIT_FAILURE);
    }

    if (listen(server_fd, BACKLOG) < 0) {
        perror("listen");
        exit(EXIT_FAILURE);
    }

    printf("Server started on port %d using %s mode...\n", PORT, mode ? "pthread" : "fork");

    while (1) {
        client_fd = accept(server_fd, (struct sockaddr *)&addr, &addrlen);
        if (client_fd < 0) {
            perror("accept");
            continue;
        }

        if (mode == 0) {
            pid_t pid = fork();
            if (pid == 0) {
                close(server_fd);
                handle_client(client_fd);
                exit(0);
            } else {
                close(client_fd);
            }
        } else {
            pthread_t tid;
            int *fd_copy = malloc(sizeof(int));
            *fd_copy = client_fd;
            pthread_create(&tid, NULL, thread_worker, fd_copy);
            pthread_detach(tid);
        }
    }

    close(server_fd);
    return 0;
}
