#include "commom.h"

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <string.h>
#include <arpa/inet.h>
#include <unistd.h>

#define BUFSZ 1024



int main(int argc, char **argv)
{
    int equip[] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
    int id = -1;

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
    if(handleMessage_C(buf, equip, s, &id)){
        logExit("Connection error.\n");
    }
    
    while(1){
        //Wait for keybord command
        if(readInput(s, id, equip)){
            continue;
        }
        //Wait server response
        int count; unsigned total = 0;
        count = read(s, buf + total, BUFSIZ - total);
        total += count;
        handleMessage_C(buf, equip, s, &id);
    }
    
}