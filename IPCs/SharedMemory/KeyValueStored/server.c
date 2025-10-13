#define _GNU_SOURCE
#include <sys/socket.h>
#include <sys/un.h>
#include <sys/stat.h>
#include <signal.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define SOCKET_PATH "/tmp/uds-demo.sock"
#define BACKLOG 5
#define BUF_SIZE 1024
#define MAX_ITEMS 128
#define KEY_LEN 64
#define VAL_LEN 256

static int listen_fd = -1;

typedef struct {
    char key[KEY_LEN];
    char val[VAL_LEN];
} KVPair;

static KVPair store[MAX_ITEMS];
static int store_count = 0;

static void die(const char *msg) {
    perror(msg);
    exit(EXIT_FAILURE);
}

static void cleanup(void) {
    if (listen_fd != -1)
        close(listen_fd);
    unlink(SOCKET_PATH);
}

static void on_signal(int sig) {
    (void)sig;
    exit(0);
}

static ssize_t read_line(int fd, char *buf, size_t maxlen) {
    size_t n = 0;
    while (n + 1 < maxlen) {
        char c;
        ssize_t r = read(fd, &c, 1);
        if (r == 0) break;
        if (r < 0) {
            if (errno == EINTR) continue;
            return -1;
        }
        buf[n++] = c;
        if (c == '\n') break;
    }
    buf[n] = '\0';
    return (ssize_t)n;
}

static int write_all(int fd, const void *buf, size_t len) {
    const char *p = buf;
    while (len > 0) {
        ssize_t w = write(fd, p, len);
        if (w < 0) {
            if (errno == EINTR) continue;
            return -1;
        }
        p += w;
        len -= (size_t)w;
    }
    return 0;
}

static void set_value(const char *k, const char *v) {
    for (int i = 0; i < store_count; ++i) {
        if (strcmp(store[i].key, k) == 0) {
            strncpy(store[i].val, v, VAL_LEN - 1);
            store[i].val[VAL_LEN - 1] = '\0';
            return;
        }
    }
    if (store_count < MAX_ITEMS) {
        strncpy(store[store_count].key, k, KEY_LEN - 1);
        store[store_count].key[KEY_LEN - 1] = '\0';
        strncpy(store[store_count].val, v, VAL_LEN - 1);
        store[store_count].val[VAL_LEN - 1] = '\0';
        store_count++;
    }
}

static const char *get_value(const char *k) {
    for (int i = 0; i < store_count; ++i)
        if (strcmp(store[i].key, k) == 0)
            return store[i].val;
    return NULL;
}

int main(void) {
    atexit(cleanup);

    struct sigaction sa = {0};
    sa.sa_handler = on_signal;
    sigaction(SIGINT, &sa, NULL);
    sigaction(SIGTERM, &sa, NULL);

    umask(077);
    unlink(SOCKET_PATH);

    listen_fd = socket(AF_UNIX, SOCK_STREAM, 0);
    if (listen_fd == -1) die("socket");

    struct sockaddr_un addr;
    memset(&addr, 0, sizeof(addr));
    addr.sun_family = AF_UNIX;
    strncpy(addr.sun_path, SOCKET_PATH, sizeof(addr.sun_path) - 1);

    if (bind(listen_fd, (struct sockaddr *)&addr, sizeof(addr)) == -1) die("bind");
    if (chmod(SOCKET_PATH, 0600) == -1) die("chmod");
    if (listen(listen_fd, BACKLOG) == -1) die("listen");

    fprintf(stderr, "[server] listening on %s\n", SOCKET_PATH);

    for (;;) {
        int client_fd = accept(listen_fd, NULL, NULL);
        if (client_fd == -1) {
            if (errno == EINTR) continue;
            die("accept");
        }

        write_all(client_fd, "Mini KV Store ready\n", 20);

        char buf[BUF_SIZE];
        while (1) {
            ssize_t n = read_line(client_fd, buf, sizeof(buf));
            if (n <= 0) break;

            buf[strcspn(buf, "\r\n")] = '\0';

            if (strncmp(buf, "SET ", 4) == 0) {
                char key[KEY_LEN], val[VAL_LEN];
                if (sscanf(buf + 4, "%63s %255[^\n]", key, val) == 2) {
                    set_value(key, val);
                    write_all(client_fd, "OK\n", 3);
                } else {
                    write_all(client_fd, "ERR usage: SET <key> <value>\n", 29);
                }
            } else if (strncmp(buf, "GET ", 4) == 0) {
                char key[KEY_LEN];
                if (sscanf(buf + 4, "%63s", key) == 1) {
                    const char *v = get_value(key);
                    if (v) {
                        char out[VAL_LEN + KEY_LEN + 8];
                        int m = snprintf(out, sizeof(out), "VALUE %s\n", v);
                        if (m > 0) write_all(client_fd, out, (size_t)m);
                    } else {
                        write_all(client_fd, "NOTFOUND\n", 9);
                    }
                } else {
                    write_all(client_fd, "ERR usage: GET <key>\n", 21);
                }
            } else if (strcmp(buf, "EXIT") == 0) {
                write_all(client_fd, "BYE\n", 4);
                break;
            } else {
                write_all(client_fd, "ERR unknown command\n", 20);
            }
        }

        close(client_fd);
    }

    

    return 0;
}