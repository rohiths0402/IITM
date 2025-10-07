#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/un.h>

#define SOCKET_PATH "/tmp/kv_socket"
#define BUF_SIZE 512

int main() {
    int sockfd;
    struct sockaddr_un addr;
    char buf[BUF_SIZE];

    sockfd = socket(AF_UNIX, SOCK_STREAM, 0);
    if (sockfd < 0) { perror("socket"); exit(1); }

    memset(&addr, 0, sizeof(addr));
    addr.sun_family = AF_UNIX;
    strncpy(addr.sun_path, SOCKET_PATH, sizeof(addr.sun_path) - 1);

    if (connect(sockfd, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
        perror("connect"); exit(1);
    }

    printf("Connected to KV Server. Use:\n  SET key value\n  GET key\n> ");
    while (fgets(buf, sizeof(buf), stdin)) {
        write(sockfd, buf, strlen(buf));
        ssize_t n = read(sockfd, buf, sizeof(buf) - 1);
        if (n > 0) {
            buf[n] = '\0';
            printf("Response: %s", buf);
        }
        printf("> ");
    }

    close(sockfd);
    return 0;
}