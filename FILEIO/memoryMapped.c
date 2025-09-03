#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

static void exitWithError(const char *context){
    perror(context);
    exit(EXIT_FAILURE);
}

static void scanForPattern(const char *buffer, size_t size, const char *pattern){
    size_t patternLen = strlen(pattern);
    if(patternLen == 0 || size < patternLen) return;
    for(size_t offset = 0; offset <= size - patternLen; ++offset){
        if(memcmp(buffer + offset, pattern, patternLen) == 0){
            printf("Match found at byte offset: %zu\n", offset);
        }
    }
}

int main(int argc, char *argv[]){
    if(argc != 3){
        fprintf(stderr, "Usage: %s <filepath> <pattern>\n", argv[0]);
        return EXIT_FAILURE;
    }
    const char *filepath = argv[1];
    const char *pattern = argv[2];
    int fd = open(filepath, O_RDONLY);
    if(fd < 0){
        exitWithError("open");
    } 
    struct stat fileStat;
    if(fstat(fd, &fileStat) < 0){
        close(fd);
        exitWithError("fstat");
    }
    size_t fileSize = fileStat.st_size;
    if(fileSize == 0){
        close(fd);
        fprintf(stderr, "File is empty\n");
        return EXIT_FAILURE;
    }
    const char *fileData = mmap(NULL, fileSize, PROT_READ, MAP_PRIVATE, fd, 0);
    if(fileData == MAP_FAILED){
        close(fd);
        exitWithError("mmap");
    }
    scanForPattern(fileData, fileSize, pattern);
    munmap((void *)fileData, fileSize);
    close(fd);
    return EXIT_SUCCESS;
}