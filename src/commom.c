#include "commom.h"


void logExit(const char *msg)
{
    perror(msg);
    exit(EXIT_FAILURE);
}

int addrParse(const char *addrStr, const char *portStr, struct sockaddr_storage *storage)
{
    if (addrStr == NULL || portStr == NULL)
    {
        return -1;
    }
    uint16_t port = (uint16_t)atoi(portStr);
    if (port == 0)
        return -1;
    port = htons(port);

    struct in_addr inaddr4;
    if (inet_pton(AF_INET, addrStr, &inaddr4.s_addr))
    {
        struct sockaddr_in *addr4 = (struct sockaddr_in *)storage;
        addr4->sin_family = AF_INET;
        addr4->sin_port = port;
        addr4->sin_addr = inaddr4;
        return 0;
    }

    return -1;
}

void addrToStr(const struct sockaddr *addr, char *str, size_t strSize)
{
    int version;
    char addrStr[INET6_ADDRSTRLEN + 1] = "";
    uint16_t port;
    if (addr->sa_family == AF_INET)
    {
        struct sockaddr_in *addr4 = (struct sockaddr_in *)addr;
        version = 4;
        if (!inet_ntop(AF_INET, &(addr4->sin_addr), addrStr, INET_ADDRSTRLEN))
        {
            logExit("ntop");
        }
        port = ntohs(addr4->sin_port);
    }
    else
    {
        logExit("unknown protocol family.");
    }
    if (str)
    {
        snprintf(str, strSize, "IPv%d %s %hu", version, addrStr, port);
    }
}

int ServerSockInit(const char *proto, const char *portStr,
                   struct sockaddr_storage *storage)
{
    if (proto == NULL || portStr == NULL)
    {
        return -1;
    }
    uint16_t port = (uint16_t)atoi(portStr);
    if (port == 0){return -1;}
    port = htons(port);

    memset(storage, 0, sizeof(*storage));
    if (strcmp(proto, "v4") == 0)
    {
        struct sockaddr_in *addr4 = (struct sockaddr_in *)storage;
        addr4->sin_family = AF_INET;
        addr4->sin_port = port;
        addr4->sin_addr.s_addr = INADDR_ANY;
        return 0;
    }
    else
    {
        return -1;
    }
}


/*Funcao de tratamento de mensagens pelo servidor. Recebe como parâmetro uma string enviada pelo cliente
e a partir dos dados nela, realiza a acao necessaria. Retorna uma mensagem de resposta que deve ser enviada
ao cliente.*/
void handleMessage_S(char* data, int* equipaments, int* csockets, int thisSocket){
    char* sub;
    char buf[BUFSIZ];
    memset(buf, 0, BUFSIZ);

    if((sub = strtok(data, " "))!= NULL){
        //REQ_ADD
        if(strcmp(sub, "01")== 0){
            int r = addEquip(equipaments, csockets, thisSocket);
            if(r){
                snprintf(buf, sizeof(buf), "03 %d", r);
                sendToAll(buf, equipaments,csockets);
                memset(buf, 0, BUFSIZ);
                snprintf(buf, sizeof(buf), "03 _%d", r);
                send(thisSocket, buf, strlen(buf) + 1, 0);
                listEquips(equipaments, thisSocket);
            }
            else{
                handleError(4, thisSocket);
            }
        }
        //REQ_RM
        else if(strcmp(sub, "02")== 0){
            sub = strtok(NULL, " ");
            if(sub == NULL){return NULL;}
            int id = atoi(sub);
            int r = removeEquip(id, equipaments, csockets);
            if(r){
                snprintf(buf, sizeof(buf), "08 01:Success");
                send(thisSocket, buf, strlen(buf) + 1, 0);
                printf("Equipament IdEq %d removed\n", id);
                memset(buf, 0, BUFSIZ);
                snprintf(buf, sizeof(buf), "02 %d", r);
                sendToAll(buf, equipaments,csockets);
            }else{
                handleError(1, thisSocket);
            }
        }
        //REQ_INF
        else if(strcmp(sub, "05")== 0){
            sub = strtok(NULL, " ");
            if(sub == NULL){return NULL;}
            int originId = atoi(sub);
            sub = strtok(NULL, " ");
            if(sub == NULL){return NULL;}
            int destId = atoi(sub);
            if(!equipaments[originId - 1]){
                printf("Equipament IdEq %d not found", originId);
                handleError(2, thisSocket);
            }
            else if(!equipaments[destId - 1]){
                printf("Equipament IdEq %d not found", destId);
                handleError(3, thisSocket);
            }
            else{
                snprintf(buf, sizeof(buf), "05 %d %d", originId, destId);
                int destSocket = csockets[destId - 1];
                send(destSocket, buf, strlen(buf) + 1, 0);
            }

        }
        //RES_INF
        else if(strcmp(sub, "06")== 0){
            sub = strtok(NULL, " ");
            if(sub == NULL){return NULL;}
            int originId = atoi(sub);
            sub = strtok(NULL, " ");
            if(sub == NULL){return NULL;}
            int destId = atoi(sub);
            if(!equipaments[originId - 1]){
                printf("Equipament IdEq %d not found", originId);
                handleError(2, thisSocket);
            }
            else if(!equipaments[destId - 1]){
                printf("Equipament IdEq %d not found", destId);
                handleError(3, thisSocket);
            }
            else{
                sub = strtok(NULL, " ");
                if(sub == NULL){return NULL;}
                float value = atof(sub);
                snprintf(buf, sizeof(buf), "06 %d %d %f", originId, destId, value);
                int destSocket = csockets[destId - 1];
                send(destSocket, buf, strlen(buf) + 1, 0);
            }
        }
        else{
            printf("Unknown instruction\n");
        }
    }else{
        printf("Unknown instruction\n");
    }
    return NULL;
}

//Adds a equipament to the database
int addEquip(int* equipaments, int* csockets, int thisSocket){
    for(int i = 0; i< 10; i++){ 
        if(equipaments[i] != -1){
            equipaments[i] = 1;
            csockets[i] = thisSocket;
            int id = i + 1;
            return id;
        }
    }
    return 0;
}

//Remove an equipment of the database
int removeEquip(int id, int* equipaments, int*csockets){
    if(equipaments[id - 1]){
        equipaments[id - 1] = 0;
        csockets[id - 1] = 0;
        return 1;
    }else{
        return 0;
    }
}

//Send a message to all clients connected
void sendToAll(char* buf, int* equipaments,int* csockets){
    for(int i =0; i< 10; i++){
        if(equipaments[i]){
            send(csockets[i], buf, strlen(buf) + 1, 0);
        }
    }
}

//Send a message of all equipaments on database to a equipament
void listEquips(int* equipaments, int thisSocket){
    int cx;
    char buf[BUFSIZ];
    cx = snprintf(buf, BUFSIZ, "04");
    for(int i =0; i < 10; i++){
        if(equipaments[i]){
            cx = snprintf(buf + cx, BUFSIZ - cx, " %d", i + 1);
        }
    }
    send(thisSocket, buf, strlen(buf) + 1, 0);
}


//Funcao de tratamento de mensagem recebida pelo cliente
handleMessage_C(char* data, int* equipaments){
    char* sub;
    char buf[BUFSIZ];
    memset(buf, 0, BUFSIZ);

    if((sub = strtok(data, " "))!= NULL){
        //REQ_ADD
        if(strcmp(sub, "01")== 0){

        }
    }
}