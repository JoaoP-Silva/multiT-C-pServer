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


struct client_data{
    int sock;
    int* csockets;
    struct sockaddr_storage storage;
    int* equip;
};


void* client_thread(void* data){

    struct client_data* cdata = (struct client_data*) data;
    struct sockaddr *caddr = (struct sockaddr *)(&cdata->storage);

    char caddrStr[BUFSIZ];
        addrToStr(caddr, caddrStr, BUFSIZ);
        //printf("Connection from %s\n", caddrStr);
        char buf[BUFSIZ];
        while(1){
            memset(buf, 0, BUFSIZ);
            read(cdata->sock, buf, BUFSIZ - 1);
            if(handleMessage_S(buf, cdata->equip, cdata->csockets, cdata->sock)){
                break;
            }
        }
    pthread_exit(EXIT_SUCCESS);    
}

int equip[] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
int csockets[] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0};


int main(int argc, char **argv)
{

    struct sockaddr_storage storage;
    if (ServerSockInit(argv[1], &storage) != 0)
    {
        logExit("Parameter error");
    }

    int s;
    s = socket(storage.ss_family, SOCK_STREAM, 0);
    if (s == -1)
    {
        logExit("Error at socket creation");
    }

    int enable = 1;
    if (setsockopt(s, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(int)) != 0)
    {
        logExit("Setsockopt");
    }

    struct sockaddr *addr = (struct sockaddr *)(&storage);
    if (bind(s, addr, sizeof(storage)) != 0)
    {
        logExit("Error at bind");
    }
    if (listen(s, 10) != 0)
    {
        logExit("Error at listen");
    }
    char addrStr[BUFSIZ];
    addrToStr(addr, addrStr, BUFSIZ);
    printf("Bound to %s, waiting connection\n", addrStr);

    while (1)
    {
        struct sockaddr_storage cstorage;
        struct sockaddr *caddr = (struct sockaddr *)(&cstorage);
        socklen_t caddrlen = sizeof(cstorage);
        int csock = accept(s, caddr, &caddrlen);
        if (csock == -1)
        {
            logExit("Accept error");
        }

        struct client_data* cdata = malloc(sizeof(*cdata));
        cdata->sock = csock;
        cdata->csockets = csockets;
        cdata->equip = equip;
        memcpy(&(cdata->storage), &cstorage, sizeof(cstorage));

        pthread_t tid;
        pthread_create(&tid, NULL, client_thread, cdata);
    }
    exit(EXIT_SUCCESS);
}


