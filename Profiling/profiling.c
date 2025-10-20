#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
 
int main() {
    int fd = open("sample.txt", O_RDONLY);
    char buf[100];
    read(fd, buf, 50);
    write(1, buf, 50);
    close(fd);
    return 0;
}