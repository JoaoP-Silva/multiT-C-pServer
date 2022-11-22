#include "commom.h"

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <string.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <pthread.h>

#define BUFSZ 1024

struct eqdata
{
    int* id;
    int* equip;
    int socket;
};

void* equip_thread(void* data){
    struct eqdata* eqdata = (struct eqdata*) data;
    char buf[BUFSIZ];
    while(1){
        memset(buf, 0, BUFSIZ);
        recv(eqdata->socket, buf, BUFSIZ, 0);
        char aux[BUFSIZ];
        int cpyB = 0, cpyA = 0;
        memset(aux, 0, BUFSIZ);

        while(buf[cpyB] != '\0' || buf[cpyB + 1] != '\0'){
            while(buf[cpyB] != '\0'){
            memcpy(aux + cpyA, buf + cpyB, 1);
            cpyB++; cpyA++;
            }   
            handleMessage_C(aux, eqdata->equip, eqdata->socket, eqdata->id);
            memset(aux, 0, BUFSIZ);
            cpyB++; cpyA = 0;
        }
    }
    
}


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
    //printf("Connected to %s\n", addrStr);
    char buf[BUFSIZ];
    memset(buf, 0, BUFSIZ);
    snprintf(buf, BUFSIZ, "01");
    send(s, buf, strlen(buf) + 1, 0);
    memset(buf, 0, BUFSIZ);
    recv(s, buf, BUFSIZ, 0);
    char aux[BUFSIZ];
    int cpyB = 0, cpyA = 0;
    memset(aux, 0, BUFSIZ);

    while(buf[cpyB] != '\0' || buf[cpyB + 1] != '\0'){
        while(buf[cpyB] != '\0'){
        memcpy(aux + cpyA, buf + cpyB, 1);
        cpyB++; cpyA++;
        }   
        handleMessage_C(aux, equip, s, &id);
        memset(aux, 0, BUFSIZ);
        cpyB++; cpyA = 0;
    }

    struct eqdata* edata = malloc(sizeof(*edata));
    edata->equip = equip;
    edata->id = &id;
    edata->socket = s;

    pthread_t tid;
    pthread_create(&tid, NULL, equip_thread, edata);
    while(1){
        memset(buf, 0, BUFSIZ);
        memset(aux, 0, BUFSIZ);
        cpyB = 0, cpyA = 0;
        //Wait for keybord command
        if(readInput(s, id, equip)){
            break;
        }
    }
    return 0;
}