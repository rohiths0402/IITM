#ifndef CHAT_H
#define CHAT_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ipc.h>
#include <sys/msg.h>

#define MSGKEY 1234
#define MAX_TEXT 256

struct msg {
    long mtype;
    char text[MAX_TEXT];
};

#endif