#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/time.h>

#define PORT 8080
#define BACKLOG 10
#define BUF_SIZE 1024

void handle_client(int client_fd) {
    char buffer[BUF_SIZE];
    int bytes = read(client_fd, buffer, BUF_SIZE);
    if (bytes > 0) {
        buffer[bytes] = '\0';
        printf("Received: %s", buffer);
        write(client_fd, "OK\n", 3);
    }
    close(client_fd);
}

int main() {
    int server_fd, client_fd;
    struct sockaddr_in addr;
    socklen_t addrlen = sizeof(addr);

    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd < 0) {
        perror("socket");
        exit(1);
    }

    int opt = 1;
    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(PORT);

    if (bind(server_fd, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
        perror("bind");
        exit(1);
    }

    if (listen(server_fd, BACKLOG) < 0) {
        perror("listen");
        exit(1);
    }

    printf("Server started on port %d\n", PORT);

    for (int i = 0; i < 10; i++) { 
        client_fd = accept(server_fd, (struct sockaddr *)&addr, &addrlen);
        if (client_fd >= 0) {
            handle_client(client_fd);
        }
    }

    close(server_fd);
    printf("Server exiting with status 1\n");
    return 1;
}
