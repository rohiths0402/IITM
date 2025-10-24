#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <poll.h>

#define MAX_FILES 3
#define BUFFER_SIZE 64

int main() {
    const char *files[MAX_FILES] = {"file1.txt", "file2.txt", "file3.txt"};
    int fds[MAX_FILES];
    struct pollfd pfds[MAX_FILES];
    char buffer[BUFFER_SIZE];
    for (int i = 0; i < MAX_FILES; i++) {
        fds[i] = open(files[i], O_RDONLY);
        if (fds[i] < 0) {
            perror(files[i]);
            return 1;
        }
        pfds[i].fd = fds[i];
        pfds[i].events = POLLIN; 
    }

    int active = MAX_FILES;
    while (active > 0) {
        int ret = poll(pfds, MAX_FILES, 5000); 
        if (ret < 0) {
            perror("poll");
            break;
        } else if (ret == 0) {
            printf("Timeout, no data ready\n");
            continue;
        }

        for (int i = 0; i < MAX_FILES; i++) {
            if (pfds[i].revents & POLLIN) {
                ssize_t bytes = read(pfds[i].fd, buffer, sizeof(buffer));
                if (bytes <= 0) {
                    close(pfds[i].fd);
                    pfds[i].fd = -1; 
                    active--;
                    printf("%s closed\n", files[i]);
                } else {
                    printf("%s: Read %ld bytes\n", files[i], bytes);
                }
            }
        }
    }

    return 0;
}
