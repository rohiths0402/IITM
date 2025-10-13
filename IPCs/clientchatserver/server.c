#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define PORT 9000
#define MAX_CLIENTS FD_SETSIZE
#define BUFFER_SIZE 1024

int set_nonblocking(int fd) {
    int flags = fcntl(fd, F_GETFL, 0);
    return fcntl(fd, F_SETFL, flags | O_NONBLOCK);
}

void broadcast(int sender_fd, fd_set *clients, int max_fd, const char *msg, int len) {
    for (int fd = 0; fd <= max_fd; fd++) {
        if (FD_ISSET(fd, clients) && fd != sender_fd) {
            send(fd, msg, len, 0);
        }
    }
}

int main() {
    int server_fd, client_fd, max_fd;
    struct sockaddr_in server_addr, client_addr;
    socklen_t addr_len = sizeof(client_addr);
    char buffer[BUFFER_SIZE];

    fd_set master_set, read_set;

    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd < 0) { perror("socket"); exit(EXIT_FAILURE); }

    set_nonblocking(server_fd);

    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(PORT);

    if (bind(server_fd, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        perror("bind"); exit(EXIT_FAILURE);
    }

    if (listen(server_fd, SOMAXCONN) < 0) {
        perror("listen"); exit(EXIT_FAILURE);
    }

    FD_ZERO(&master_set);
    FD_SET(server_fd, &master_set);
    max_fd = server_fd;

    printf("Chat server started on port %d...\n", PORT);

    while (1) {
        read_set = master_set;

        if (select(max_fd + 1, &read_set, NULL, NULL, NULL) < 0) {
            perror("select"); continue;
        }

        for (int fd = 0; fd <= max_fd; fd++) {
            if (!FD_ISSET(fd, &read_set)) continue;

            if (fd == server_fd) {
                client_fd = accept(server_fd, (struct sockaddr*)&client_addr, &addr_len);
                if (client_fd < 0) {
                    if (errno != EWOULDBLOCK) perror("accept");
                    continue;
                }

                set_nonblocking(client_fd);
                FD_SET(client_fd, &master_set);
                if (client_fd > max_fd) max_fd = client_fd;

                printf("New client connected: %s:%d (fd %d)\n",
                       inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port), client_fd);
            } else {
                int bytes = recv(fd, buffer, sizeof(buffer), 0);
                if (bytes <= 0) {
                    if (bytes == 0) printf("Client disconnected (fd %d)\n", fd);
                    else perror("recv");

                    close(fd);
                    FD_CLR(fd, &master_set);
                } else {
                    buffer[bytes] = '\0';
                    broadcast(fd, &master_set, max_fd, buffer, bytes);
                }
            }
        }
    }

    close(server_fd);
    return 0;
}