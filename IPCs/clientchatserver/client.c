#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/select.h>

#define SERVER_IP "127.0.0.1"
#define SERVER_PORT 9000
#define BUFFER_SIZE 1024

int main() {
    int sockfd;
    struct sockaddr_in server_addr;
    char buffer[BUFFER_SIZE];
    fd_set read_fds;

    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) { perror("socket"); exit(EXIT_FAILURE); }

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(SERVER_PORT);
    inet_pton(AF_INET, SERVER_IP, &server_addr.sin_addr);

    if (connect(sockfd, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        perror("connect"); exit(EXIT_FAILURE);
    }

    printf("Connected to chat server. Type messages:\n");

    while (1) {
        FD_ZERO(&read_fds);
        FD_SET(sockfd, &read_fds);
        FD_SET(STDIN_FILENO, &read_fds);

        if (select(sockfd + 1, &read_fds, NULL, NULL, NULL) < 0) {
            perror("select"); break;
        }

        if (FD_ISSET(STDIN_FILENO, &read_fds)) {
            if (fgets(buffer, BUFFER_SIZE, stdin) == NULL) break;
            send(sockfd, buffer, strlen(buffer), 0);
        }

        if (FD_ISSET(sockfd, &read_fds)) {
            int bytes = recv(sockfd, buffer, BUFFER_SIZE - 1, 0);
            if (bytes <= 0) break;
            buffer[bytes] = '\0';
            printf("Message: %s", buffer);
        }
    }

    close(sockfd);
    return 0;
}