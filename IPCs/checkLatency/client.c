#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/time.h>

#define SERVER_PORT 8080
#define REQUESTS 10
#define BUF_SIZE 1024

double get_time_ms() {
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return tv.tv_sec * 1000.0 + tv.tv_usec / 1000.0;
}

int main() {
    double total_latency = 0.0;

    for (int i = 0; i < REQUESTS; i++) {
        int sock = socket(AF_INET, SOCK_STREAM, 0);
        struct sockaddr_in server_addr;
        server_addr.sin_family = AF_INET;
        server_addr.sin_port = htons(SERVER_PORT);
        server_addr.sin_addr.s_addr = inet_addr("127.0.0.1");

        double start = get_time_ms();
        if (connect(sock, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
            perror("connect");
            exit(1);
        }

        char msg[] = "ping\n";
        send(sock, msg, strlen(msg), 0);

        char buffer[BUF_SIZE];
        int bytes = recv(sock, buffer, BUF_SIZE, 0);
        double end = get_time_ms();

        if (bytes > 0) {
            buffer[bytes] = '\0';
            printf("Server replied: %s", buffer);
        }

        close(sock);

        double latency = end - start;
        printf("Request %d latency: %.3f ms\n", i + 1, latency);
        total_latency += latency;
    }

    printf("Average latency: %.3f ms/request\n", total_latency / REQUESTS);

    return 1; // <-- exit with 1 for strace
}
