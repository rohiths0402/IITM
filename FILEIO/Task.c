#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <fcntl.h>
#include <unistd.h>
#include <time.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>

#define MAX_FILES 10

typedef struct{
    char *FileName;
    char *content;
    long long completionTime;
    size_t size;
}FileResult;

FileResult results[MAX_FILES];
int file_count = 0;

long long getNanotime(){
    struct timespec ts;
    clockGettime(CLOCK_MONOTONIC,&ts);
    return(long long) ts.tv_sec * 1000000000LL + ts.tv_nsec;
}

void *readFile(void *arg){
    int idx = *(int *)arg;
    char *FileName = results[idx].FileName;
    int fd = open(FileName, O_RDONLY);
    if(fd < 0){
        perror("open failed");
        pthread_exit(NULL);
    }
    struct stat st;
    if(fstat(fd, &st) != 0){
        perror("fstat failed");
        close(fd);
        pthread_exit(NULL);
    }
    results[idx].size = st.st_size;
    results[idx].content = malloc(results[idx].size + 1);
    if(!results[idx].content){
        perror("malloc failed");
        close(fd);
        pthread_exit(NULL);
    }
    ssize_t read_bytes = read(fd, results[idx].content, results[idx].size);
    if(read_bytes < 0){
        perror("read failed");
        free(results[idx].content);
        close(fd);
        pthread_exit(NULL);
    }
    results[idx].content[read_bytes] = '\0';
    close(fd);
    results[idx].completionTime = getNanotime();
    pthread_exit(NULL);
}

int compareResult(const void *a, const void *b){
    FileResult *fa =(FileResult *)a;
    FileResult *fb =(FileResult *)b;
    if(fa->completionTime < fb->completionTime){
        return -1;
    }
    if(fa->completionTime > fb->completionTime){
        return 1;
    }
    return 0;
}

void merge_files(){
    qsort(results, file_count, sizeof(FileResult), compareResult);
    printf("\n--- Merged Output(by completion time) ---\n\n");
    for(int i = 0;i< file_count;i++){
        printf(">>> From file: %s\n", results[i].FileName);
        printf("%s\n", results[i].content);
    }
}

int main(){
    printf("Enter number of files(max %d): ", MAX_FILES);
    scanf("%d", &file_count);
    if(file_count <= 0 || file_count > MAX_FILES){
        printf("Invalid file count\n");
        return 1;
    }
    for(int i = 0;i< file_count;i++){
        char buffer[256];
        printf("Enter path for file %d: ",i + 1);
        scanf("%s", buffer);
        results[i].FileName = strdup(buffer);
        results[i].content = NULL;
        results[i].completionTime = 0;
        results[i].size = 0;
    }
    pthread_t threads[MAX_FILES];
    int indices[MAX_FILES];
    for(int i = 0;i< file_count;i++){
        indices[i] = i;
        if(pthread_create(&threads[i], NULL, readFile, &indices[i]) != 0){
            perror("pthread_create failed");
            return 1;
        }
    }
    for(int i = 0;i<file_count;i++){
        pthread_join(threads[i], NULL);
    }
    merge_files();
    for(int i = 0;i<file_count;i++){
        free(results[i].content);
        free(results[i].FileName);
    }
    return 0;
}