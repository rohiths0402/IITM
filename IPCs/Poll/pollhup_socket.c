#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <poll.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>

#define PORT 8080

int main() {
    int pid;
    struct sockaddr_in addr;

    pid = fork();
    if (pid < 0) {
        perror("fork");
        exit(EXIT_FAILURE);
    }

    if (pid == 0) {
        sleep(1); 
        int sockfd = socket(AF_INET, SOCK_STREAM, 0);
        if (sockfd < 0) {
            perror("Child: socket");
            exit(EXIT_FAILURE);
        }
        addr.sin_family = AF_INET;
        addr.sin_port = htons(PORT);
        addr.sin_addr.s_addr = inet_addr("127.0.0.1");
        if (connect(sockfd, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
            perror("Child: connect");
            exit(EXIT_FAILURE);
        }
        char msg[] = "Hello! This is child process";
        write(sockfd, msg, strlen(msg));
        close(sockfd);
        return 0;
    } 
    else {
        int server_fd = socket(AF_INET, SOCK_STREAM, 0);
        if (server_fd < 0) {
            perror("Parent: socket");
            exit(EXIT_FAILURE);
        }

        int opt = 1;
        setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

        addr.sin_family = AF_INET;
        addr.sin_addr.s_addr = INADDR_ANY;
        addr.sin_port = htons(PORT);

        if (bind(server_fd, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
            perror("Parent: bind");
            exit(EXIT_FAILURE);
        }

        if (listen(server_fd, 5) < 0) {
            perror("Parent: listen");
            exit(EXIT_FAILURE);
        }

        printf("Parent: waiting for connection...\n");

        int client_fd = accept(server_fd, NULL, NULL);
        if (client_fd < 0) {
            perror("Parent: accept");
            exit(EXIT_FAILURE);
        }

        struct pollfd pfd;
        pfd.fd = client_fd;
        pfd.events = POLLIN;

        char buffer[100];

        while (1) {
            int ret = poll(&pfd, 1, 500);
            if (ret < 0) {
                perror("Poll");
                exit(EXIT_FAILURE);
            }
            if (ret == 0) {
            
                continue;
            }

            if (pfd.revents & POLLIN) {
                int n = read(client_fd, buffer, sizeof(buffer) - 1);
                if (n > 0) {
                    buffer[n] = '\0';
                    printf("The child wrote: %s\n", buffer);
                } else if (n == 0) {
                    printf("Client closed connection.\n");
                    break;
                }
            }

            if (pfd.revents & (POLLHUP | POLLERR)) {
                printf("POLLHUP or POLLERR detected! Closing.\n");
                break;
            }
        }

           

        close(client_fd);
        close(server_fd);
    }

    return 0;
}
