#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/time.h>

#define SERVER_PORT 8080
#define REQUESTS 1000
#define BUF_SIZE 1024

double get_time_ms() {
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return tv.tv_sec * 1000.0 + tv.tv_usec / 1000.0;
}

int main() {
    double start = get_time_ms();

    for (int i = 0; i < REQUESTS; i++) {
        int sock = socket(AF_INET, SOCK_STREAM, 0);
        struct sockaddr_in server_addr;
        server_addr.sin_family = AF_INET;
        server_addr.sin_port = htons(SERVER_PORT);
        server_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
        connect(sock, (struct sockaddr *)&server_addr, sizeof(server_addr));
        char msg[] = "ping\n";
        send(sock, msg, strlen(msg), 0);
        char buffer[BUF_SIZE];
        recv(sock, buffer, BUF_SIZE, 0);
        close(sock);
    }
    double end = get_time_ms();
    printf("Total time: %.2f ms for %d requests\n", end - start, REQUESTS);
    printf("Average latency: %.2f ms/request\n", (end - start) / REQUESTS);
    return 0;
}
