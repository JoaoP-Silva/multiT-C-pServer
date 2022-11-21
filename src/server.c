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
    int csock;
    int sock;
    struct sockaddr_storage storage;
    struct sockaddr_storage cstorage;
    int *rack1, *rack2; 
};


void* client_thread(void* data){

    struct client_data* cdata = (struct client_data*) data;
    struct sockaddr *caddr = (struct sockaddr *)(&cdata->cstorage);

    char caddrStr[BUFSIZ];
        addrToStr(caddr, caddrStr, BUFSIZ);
        printf("Connection from %s\n", caddrStr);
        char buf[BUFSIZ];
        while(1){
            memset(buf, 0, BUFSIZ);
            size_t count = read(cdata->csock, buf, BUFSIZ - 1);
            char* res = handleMessage(buf, cdata->rack1, cdata->rack2);
            if(res == NULL){
                strcpy(buf, "exit");
                count = send(cdata->sock, buf, strlen(buf) + 1, 0);
                close(cdata->csock);
                printf("Closed connection\n");
                break;
            }
            memset(buf, 0, BUFSIZ);
            strcpy(buf, res);
            count = send(cdata->csock, buf, strlen(buf) + 1, 0);
            if (count != strlen(buf) + 1)
            {
                logExit("Error at send message\n");
            }
        }
    pthread_exit(EXIT_SUCCESS);    
}



int main(int argc, char **argv)
{
    //Inicializa a lista de equipamentos
    int rack1[] = {0, 0, 0, 0};
    int rack2[] = {0, 0, 0, 0};

    struct sockaddr_storage storage;
    if (ServerSockInit(argv[1], argv[2], &storage) != 0)
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
        cdata->csock = csock;
        cdata->sock = s;
        cdata->rack1 = rack1;
        cdata->rack2 = rack2;

        memcpy(&(cdata->cstorage), &cstorage, sizeof(cstorage));
        memcpy(&(cdata->storage), &storage, sizeof(storage));

        pthread_t tid;
        pthread_create(&tid, NULL, client_thread, cdata);
    }
    exit(EXIT_SUCCESS);
}


