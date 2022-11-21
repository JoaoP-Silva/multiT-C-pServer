#include "commom.h"

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <string.h>
#include <arpa/inet.h>
#include <unistd.h>

#define BUFSZ 1024


int equip = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0};


int main(int argc, char **argv)
{

    struct sockaddr_storage storage;
    if (addrParse(argv[1], argv[2], &storage) != 0)
    {
        logExit("Parameter error\n");
    }

    int s;
    s = socket(storage.ss_family, SOCK_STREAM, 0);
    if (s == -1)
    {
        logExit("Error at socket creation\n");
    }
    struct sockaddr *addr = (struct sockaddr *)(&storage);
    if (connect(s, addr, sizeof(storage)) != 0)
    {
        logExit("Error at connect\n");
    }

    char addrStr[BUFSIZ];
    addrToStr(addr, addrStr, BUFSIZ);
    printf("Connected to %s\n", addrStr);
    char buf[BUFSIZ];
    memset(buf, 0, BUFSIZ);
    snprintf(buf, BUFSIZ, "01");
    send(s, buf, strlen(buf) + 1, 0);

    while(1){
        //Wait server response
        int count; unsigned total = 0;
        count = read(s, buf + total, BUFSIZ - total);
        total += count;
        handleMessage_C(buf, equip);

        memset(buf, 0, BUFSIZ);
        fgets(buf, BUFSIZ, stdin);
        char closebuf[BUFSIZ];
        memset(closebuf, 0, BUFSIZ);
        strcpy(closebuf, buf);
        size_t count = send(s, buf, strlen(buf) + 1, 0);
        if (count != strlen(buf) + 1)
        {
            logExit("Error at send message\n");
        }
        if(strcmp(closebuf, "exit") == 0){
            close(s);
            exit(EXIT_SUCCESS);
        }
    }
    
}