#define _POSIX_C_SOURCE 2008009L
#include "common_shm.h"

int main(int argc, char **argv)
{
    if(argc < 2)
    {
        fprintf(stderr, "Usage : %s \" message to publish\" \n", argv[0]);
        return EXIT_FAILURE;
    }

    //To check whether the file key exists and it is accessible
    if(access(FTOK_PATH, F_OK) != 0)
    {
        FILE *file = fopen(FTOK_PATH, "ab");
        if(!file)
        {
            die("fopen key file");
        }
        fclose(file);
    }

    //To generate the System V IPC key
    key_t key = ftok(FTOK_PATH, FTOK_PROJID);
    if (key == (key_t)-1)
    {
        die("ftok");
    }

    //Create/Get a shared memory segment (sized of ShmPayload)
    int shmflag = IPC_CREAT | 0666;
    int shmid = shmget(key, sizeof(ShmPayload), shmflag);
    if (shmid == -1)
    {
        die("shmget");
    }

    //attach this segment into address space
    void *address = shmat(shmid, NULL, 0);
    if(address == (void *) - 1)
    {
        die("shmat");
    }
    ShmPayload *payload = (ShmPayload *)address;

    //initialize and write data
    atomic_store_explicit(&payload->ready, 0, memory_order_relaxed);

    const char *message = argv[1];
    size_t n = strnlen(message, SHM_TEXT_CAP - 1);
    memcpy(payload->text, message, n);
    payload->text[n] = '\0';
    payload->length = n;

    //publish : Make the message visible
    atomic_thread_fence(memory_order_release);
    atomic_store_explicit(&payload->ready, 1, memory_order_release);

    //Keep the process alive, so that the reader can attach
    printf("[writer] shmid = %d key = 0x%lx wrote \"%s\" (length = %zu) ready = 1 \n"
        ,shmid, (unsigned long)key, payload->text, payload->length);
    printf("[writer] You can now run the reader...");

    sleep(5);

    //detach from the memory segment
    if(shmdt(payload) == -1)
    {
        die("shmdt (writer)");
    }
    printf("[writer] Detached.. \n");

    return EXIT_SUCCESS;
}