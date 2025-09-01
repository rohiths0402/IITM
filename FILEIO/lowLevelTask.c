#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>

void copyfiles(const char *src, const char *dest){
    char buffer[4096];
    int file = open(src, O_RDONLY);
    if(file < 0){
        perror("Failed to open source file");
        return;
    }
    int CopyFile = open(dest, O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if(CopyFile == -1){
        perror("Failed to open destination file");
        close(file);
        return;
    }
    ssize_t bytesRead;
    while((bytesRead = read(file, buffer, sizeof(buffer))) > 0){
        ssize_t bytesWritten = write(CopyFile, buffer, bytesRead);
        if(bytesWritten != bytesRead){
            perror("Write error");
            break;
        }
    }
    if(bytesRead == -1){
        perror("Read error");
    }
    close(file);
    close(CopyFile);
}

void verifyfile(const char *src, const char *dest){
    printf("Verifying byte accuracy\n");
    int src_fd = open(src, O_RDONLY);
    int dest_fd = open(dest, O_RDONLY);
    if(src_fd == -1 || dest_fd == -1){
        perror("Failed to open files for verification");
        return;
    }
    char buf1[4096], buf2[4096];
    ssize_t r1, r2;
    int mismatch = 0;
    off_t offset = 0;
    while((r1 = read(src_fd, buf1, sizeof(buf1))) > 0 && (r2 = read(dest_fd, buf2, sizeof(buf2))) > 0){
        if(r1 != r2 || memcmp(buf1, buf2, r1) != 0){
            mismatch = 1;
            printf("Mismatch detected at byte offset %lld\n",(long long)offset);
            break;
        }
        offset += r1;
    }

    if(r1 != r2){
        mismatch = 1;
        printf("File sizes differ\n");
    }

    close(src_fd);
    close(dest_fd);

    if(!mismatch){
        printf("Files are identical — byte accuracy verified.\n");
    }
}

int main(){
    copyfiles("source.txt", "destination.txt");
    verifyfile("source.txt", "destination.txt");
    return 0;
}