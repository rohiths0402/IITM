#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <ctype.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>

#define MAX_THREADS 4

typedef struct{
    char *buffer;
    long start;
    long end;
    long count;
}ThreadData;

void* count_words(void *arg){
    ThreadData *data = (ThreadData*)arg;
    long count = 0;
    int in_word = 0;
    for(long i = data->start; i < data->end; i++){
        if(isspace((unsigned char)data->buffer[i])){
            in_word = 0;
        } 
        else if(!in_word){
            in_word = 1;
            count++;
        }
    }
    data->count = count;
    return NULL;
}

long getFileSize(const char *filename){
    struct stat st;
    if(stat(filename, &st)<0){
        perror("stat");
        exit(1);
    }
    return st.st_size;
}

char* loadFile(const char *filename){
    int fd = open(filename, O_RDONLY, 0644);
    if(fd < 0){
        perror("file open");
        exit(1);
    }
    char *buffer = malloc(filesize);
    if (buffer == NULL) {
        perror("malloc allocation");
        exit(1);
    }

     if(read(fd, buffer, filesize) != filesize){
        perror("read");
        exit(1);
    }
    close(fd);
    return buffer;
}

void split_and_count(char *buffer, long filesize){
    pthread_t threads[MAX_THREADS];
    ThreadData tdata[MAX_THREADS];
    long chunk_size = filesize / MAX_THREADS;
    for(int i = 0; i < MAX_THREADS; i++){
        tdata[i].buffer = buffer;
        tdata[i].start = i * chunk_size;
        tdata[i].end = (i == MAX_THREADS - 1) ? filesize : (i + 1) * chunk_size;
        if (i > 0){
            while (tdata[i].start < filesize && 
                   !isspace((unsigned char)buffer[tdata[i].start])){
                tdata[i].start++;
            }
        }
        pthread_create(&threads[i], NULL, count_words, &tdata[i]);
    }
    long total_count = 0;
    for(int i = 0; i < MAX_THREADS; i++){
        pthread_join(threads[i], NULL);
        total_count += tdata[i].count;
    }
    printf("Total word count: %ld\n", total_count);
}

int main( int argc, char *argv[]){
    if(argc < 2){
        fprintf(stderr,"usagae:%ss <filename>\n",argv[0]);
    }
    const char *filename = argv[1];
    long filesize = getFileSize(filename);
    char *buffer = loadFile(filename, filesize);
    split_and_count(buffer, filesize);
    return 0;
    free(buffer);
    return 0;
}

