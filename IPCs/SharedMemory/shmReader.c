#define _POSIX_C_SOURCE 2008009L
#include "common_shm.h"

static void sleep_tiny(void)
{
    struct timespec ts = {.tv_sec = 0, .tv_nsec = 50 * 1000 * 1000};
    nanosleep(&ts, NULL);
}

int main(void)
{
    //Generate the key
    if(access(FTOK_PATH, F_OK) != 0)
    {
        die("Missing key file : Either execute writer first or check for the shared key file");
    }

    //To generate the System V IPC key
    key_t key = ftok(FTOK_PATH, FTOK_PROJID);
    if (key == (key_t)-1)
    {
        die("ftok");
    }

    //Look up for existing segment
    int shmid = shmget(key, sizeof(ShmPayload), 0666);
    if (shmid == -1)
    {
        die("shmget (reader). Execute the writer first");
    }

    //attach this segment into address space
    void *address = shmat(shmid, NULL, 0);
    if(address == (void *) - 1)
    {
        die("shmat");
    }
    ShmPayload *payload = (ShmPayload *)address;

    //Wait until the writer becomes ready
    printf("[reader] Attached to shmid = %d, waiting for writer to be ready.. \n", shmid);

    for(;;)
    {
        int read = atomic_load_explicit(&payload->ready, memory_order_acquire);
        if(read == 1)
        {
            break;
        }
        sleep_tiny();
    }

    //read the segment
    atomic_thread_fence(memory_order_acquire);
    printf("[reader] Message : \"%s\" (length = %zu) \n", payload->text, payload->length);

    //detach
    if(shmdt(payload) == -1)
    {
        die("shmdt (reader)");
    }

    printf("[reader] Detached..\n");

    return EXIT_SUCCESS;
}