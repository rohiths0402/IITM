#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/un.h>

#define SOCKET_PATH "/tmp/echo_socket"
#define BUFFER_SIZE 1024

int main() {
    int sockfd;
    struct sockaddr_un server_addr;
    char buffer[BUFFER_SIZE];

    sockfd = socket(AF_UNIX, SOCK_STREAM, 0);
    if (sockfd < 0) { perror("socket"); exit(EXIT_FAILURE); }

    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sun_family = AF_UNIX;
    strncpy(server_addr.sun_path, SOCKET_PATH, sizeof(server_addr.sun_path) - 1);

    if (connect(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("connect"); exit(EXIT_FAILURE);
    }

    printf("Connected to server. Type messages:\n");

    while (1) {
        printf("> ");
        fflush(stdout);
        if (fgets(buffer, BUFFER_SIZE, stdin) == NULL) break;
        send(sockfd, buffer, strlen(buffer), 0);

        int bytes = recv(sockfd, buffer, BUFFER_SIZE - 1, 0);
        if (bytes <= 0) break;

        buffer[bytes] = '\0';
        printf("Echo: %s", buffer);
    }

    close(sockfd);
    return 0;
}