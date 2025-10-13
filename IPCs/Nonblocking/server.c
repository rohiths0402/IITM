#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <sys/select.h>

#define SOCKET_PATH "/tmp/echo_socket"
#define BUFFER_SIZE 1024
#define MAX_CLIENTS FD_SETSIZE

int set_nonblocking(int fd) {
    int flags = fcntl(fd, F_GETFL, 0);
    return fcntl(fd, F_SETFL, flags | O_NONBLOCK);
}

int main() {
    int server_fd, client_fd, max_fd;
    struct sockaddr_un addr;
    fd_set master_set, read_set;
    char buffer[BUFFER_SIZE];

    unlink(SOCKET_PATH); // Remove old socket file

    server_fd = socket(AF_UNIX, SOCK_STREAM, 0);
    if (server_fd < 0) { perror("socket"); exit(EXIT_FAILURE); }

    set_nonblocking(server_fd);

    memset(&addr, 0, sizeof(addr));
    addr.sun_family = AF_UNIX;
    strncpy(addr.sun_path, SOCKET_PATH, sizeof(addr.sun_path) - 1);

    if (bind(server_fd, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
        perror("bind"); exit(EXIT_FAILURE);
    }

    if (listen(server_fd, SOMAXCONN) < 0) {
        perror("listen"); exit(EXIT_FAILURE);
    }

    FD_ZERO(&master_set);
    FD_SET(server_fd, &master_set);
    max_fd = server_fd;

    printf("UNIX echo server listening on %s...\n", SOCKET_PATH);

    while (1) {
        read_set = master_set;

        if (select(max_fd + 1, &read_set, NULL, NULL, NULL) < 0) {
            perror("select"); continue;
        }

        for (int fd = 0; fd <= max_fd; fd++) {
            if (!FD_ISSET(fd, &read_set)) continue;

            if (fd == server_fd) {
                client_fd = accept(server_fd, NULL, NULL);
                if (client_fd < 0) {
                    if (errno != EWOULDBLOCK) perror("accept");
                    continue;
                }

                set_nonblocking(client_fd);
                FD_SET(client_fd, &master_set);
                if (client_fd > max_fd) max_fd = client_fd;

                printf("New client connected (fd %d)\n", client_fd);
            } else {
                int bytes = recv(fd, buffer, sizeof(buffer), 0);
                if (bytes <= 0) {
                    if (bytes == 0) printf("Client disconnected (fd %d)\n", fd);
                    else perror("recv");

                    close(fd);
                    FD_CLR(fd, &master_set);
                } else {
                    send(fd, buffer, bytes, 0); // Echo back
                }
            }
        }
    }

    close(server_fd);
    unlink(SOCKET_PATH);
    return 0;
}