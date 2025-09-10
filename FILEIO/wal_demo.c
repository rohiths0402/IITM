#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>

#define WAL_FILE "wallog"
#define DB_FILE  "dbtxt"

static int transaction_id = 1;
void safe_write(int fd, const char *msg, int do_sync){
    if(write(fd, msg, strlen(msg)) < 0){
        perror("write");
        exit(1);
    }
    if(do_sync && fsync(fd) < 0){
        perror("fsync");
        exit(1);
    }
}

void log_to_wal(const char *key, const char *value, int do_sync){
    char buf[256];
    int fd =open(WAL_FILE, O_WRONLY | O_CREAT | O_APPEND, 0644);
    if(fd< 0){ 
        perror("open wal");
        exit(1);
    }
    snprintf(buf, sizeof(buf), "TRANSACTION %d BEGIN\n", transaction_id);
    safe_write(fd, buf, do_sync);
    snprintf(buf, sizeof(buf), "SET %s %s\n", key, value);
    safe_write(fd, buf, do_sync);
    snprintf(buf, sizeof(buf), "TRANSACTION %d COMMIT\n", transaction_id);
    safe_write(fd, buf, do_sync);
    close(fd);
}

void apply_to_db(const char *key, const char *value){
    char buf[256];
    int fd = open(DB_FILE, O_WRONLY | O_CREAT | O_APPEND, 0644);
    if(fd<0){ 
        perror("open db");
        exit(1);
    }
    snprintf(buf, sizeof(buf), "%s=%s\n", key, value);
    if(write(fd, buf, strlen(buf)) < 0){ 
        perror("db write");
    }
    fsync(fd);
    close(fd);
}

void cmd_write(const char *key, const char *value){
    log_to_wal(key, value, 1);
    apply_to_db(key, value);
    printf("Committed: %s=%s\n", key, value);
    transaction_id++;
}

void cmd_write_nosync(const char *key, const char *value){
    log_to_wal(key, value, 0);
    apply_to_db(key, value);
    printf("Committed(no sync): %s=%s\n", key, value);
    transaction_id++;
}

void cmd_crash_after_wal(const char *key, const char *value){
    log_to_wal(key, value, 1);
    printf("Simulated crash after WAL write\n");
    exit(1);
}

void cmd_recover(){
    FILE *f = fopen(WAL_FILE, "r");
    if(!f){
        printf("No WAL to recover\n");
        return;
    }
    char line[256];
    char key[128], val[128];
    int inside_tx = 0, committed = 0;
    while(fgets(line, sizeof(line), f)){
        if(strstr(line, "BEGIN")){
            inside_tx = 1;
            committed = 0;
        }
        else if(strncmp(line, "SET", 3) == 0 && inside_tx){
            sscanf(line, "SET %s %s", key, val);
        }
        else if(strstr(line, "COMMIT") && inside_tx){
            apply_to_db(key, val);
            printf("Recovered: %s=%s\n", key, val);
            inside_tx = 0;
            committed = 1;
        }
    }
    if(inside_tx && !committed){
        printf("Found incomplete transaction in WAL Ignoring\n");
    }
    fclose(f);
}

void cmd_display(){
   printf("===== WAL LOG =====\n");
    FILE *wal_fp = fopen(WAL_FILE, "r");
    if (wal_fp)
    {
        char line[MAX_LINE_LEN];
        while (fgets(line, sizeof(line), wal_fp))
        {
            printf("%s", line);
        }
        fclose(wal_fp);
    }

    printf("\n===== DB CONTENTS =====\n");
    FILE *db_fp = fopen(DB_FILE, "r");
    if (db_fp)
    {
        char line[MAX_LINE_LEN];
        while (fgets(line, sizeof(line), db_fp))
        {
            printf("%s", line);
        }
        fclose(db_fp);
}

int main(int argc, char *argv[]){
    if(argc < 2){
        printf("\nMissing command. Here's how to use this tool:\n\n");
        printf("%s write <key> <value>\n", argv[0]);
        printf("%s write-nosync <key> <value>\n", argv[0]);
        printf("%s crash-after-wal <key> <value>\n", argv[0]);
        printf("%s recover\n", argv[0]);
        printf("%s display\n", argv[0]);
        exit(1);
    }
    if(strcmp(argv[1], "write") == 0 && argc == 4){
        cmd_write(argv[2], argv[3]);
    } else if(strcmp(argv[1], "write-nosync") == 0 && argc == 4){
        cmd_write_nosync(argv[2], argv[3]);
    } else if(strcmp(argv[1], "crash-after-wal") == 0 && argc == 4){
        cmd_crash_after_wal(argv[2], argv[3]);
    } else if(strcmp(argv[1], "recover") == 0){
        cmd_recover();
    } else if(strcmp(argv[1], "display") == 0){
        cmd_display();
    } else{
        fprintf(stderr, "Invalid command\n");
        exit(1);
    }
    return 0;
}
