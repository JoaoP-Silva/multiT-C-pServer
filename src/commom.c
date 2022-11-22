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

int ServerSockInit(const char *portStr,
                   struct sockaddr_storage *storage)
{
    if (portStr == NULL)
    {
        return -1;
    }
    uint16_t port = (uint16_t)atoi(portStr);
    if (port == 0){return -1;}
    port = htons(port);

    memset(storage, 0, sizeof(*storage));

    struct sockaddr_in *addr4 = (struct sockaddr_in *)storage;
    addr4->sin_family = AF_INET;
    addr4->sin_port = port;
    addr4->sin_addr.s_addr = INADDR_ANY;
    return 0;
}


/*Funcao de tratamento de mensagens pelo servidor. Recebe como parâmetro uma string enviada pelo cliente
e a partir dos dados nela, realiza a acao necessaria. Retorna uma mensagem de resposta que deve ser enviada
ao cliente.*/
int handleMessage_S(char* data, int* equipments, int* csockets, int thisSocket){
    char* sub;
    char buf[BUFSIZ];
    memset(buf, 0, BUFSIZ);

    if((sub = strtok(data, " "))!= NULL){
        //REQ_ADD
        if(strcmp(sub, "01")== 0){
            int r = addEquip(equipments, csockets, thisSocket);
            if(r){
                printf("Equipment %d added\n", r);
                snprintf(buf, sizeof(buf), "03 %d", r);
                sendToAll(buf, equipments,csockets, r);
                memset(buf, 0, BUFSIZ);
                snprintf(buf, sizeof(buf), "03 _ %d", r);
                send(thisSocket, buf, strlen(buf) + 1, 0);
                listEquips(equipments, thisSocket);
            }
            else{
                sendError(4, thisSocket);
            }
        }
        //REQ_RM
        else if(strcmp(sub, "02")== 0){
            sub = strtok(NULL, " ");
            if(sub == NULL){return 0;}
            int id = atoi(sub);
            int r = removeEquip(id, equipments, csockets);
            if(r){
                snprintf(buf, sizeof(buf), "08 01:Success");
                send(thisSocket, buf, strlen(buf) + 1, 0);
                printf("equipment IdEq %d removed\n", id);
                memset(buf, 0, BUFSIZ);
                snprintf(buf, sizeof(buf), "02 %d", r);
                sendToAll(buf, equipments,csockets, id);
                send(thisSocket, buf, strlen(buf) + 1, 0);
                return 1;
            }else{
                sendError(1, thisSocket);
            }
        }
        //REQ_INF
        else if(strcmp(sub, "05")== 0){
            sub = strtok(NULL, " ");
            if(sub == NULL){return 0;}
            int originId = atoi(sub);
            sub = strtok(NULL, " ");
            if(sub == NULL){return 0;}
            int destId = atoi(sub);
            if(!equipments[originId - 1]){
                printf("equipment IdEq %d not found", originId);
                sendError(2, thisSocket);
            }
            else if(!equipments[destId - 1]){
                printf("equipment IdEq %d not found", destId);
                sendError(3, thisSocket);
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
            if(sub == NULL){return 0;}
            int originId = atoi(sub);
            sub = strtok(NULL, " ");
            if(sub == NULL){return 0;}
            int destId = atoi(sub);
            if(!equipments[originId - 1]){
                printf("equipment IdEq %d not found", originId);
                sendError(2, thisSocket);
            }
            else if(!equipments[destId - 1]){
                printf("equipment IdEq %d not found", destId);
                sendError(3, thisSocket);
            }
            else{
                sub = strtok(NULL, " ");
                if(sub == NULL){return 0;}
                float value = atof(sub);
                snprintf(buf, sizeof(buf), "06 %d %d %f", originId, destId, value);
                int destSocket = csockets[destId - 1];
                send(destSocket, buf, strlen(buf) + 1, 0);
            }
        }
        else if(strcmp(sub, "09")== 0){
            listEquips(equipments, thisSocket);
        }
        else{
            printf("Unknown instruction\n");
            return 0;
        }
    }else{
        printf("Unknown instruction\n");
        return 1;
    }
    return 0;
}

//Adds a equipment to the database
int addEquip(int* equipments, int* csockets, int thisSocket){
    for(int i = 0; i< 10; i++){ 
        if(!equipments[i]){
            equipments[i] = 1;
            csockets[i] = thisSocket;
            int id = i + 1;
            return id;
        }
    }
    return 0;
}

//Remove an equipment of the database
int removeEquip(int id, int* equipments, int*csockets){
    if(equipments[id - 1]){
        equipments[id - 1] = 0;
        csockets[id - 1] = 0;
        return 1;
    }else{
        return 0;
    }
}

//Send a message to all clients connected
void sendToAll(char* buf, int* equipments,int* csockets, int id){
    for(int i =0; i< 10; i++){
        if(equipments[i] && i!= id -1){
            send(csockets[i], buf, strlen(buf) + 1, 0);
        }
    }
}

//Send a message of all equipments on database to a equipment
void listEquips(int* equipments, int thisSocket){
    int cx;
    char buf[BUFSIZ];
    cx = snprintf(buf, BUFSIZ, "04");
    for(int i =0; i < 10; i++){
        if(equipments[i]){
            cx += snprintf(buf + cx, BUFSIZ - cx, " %d", i + 1);
        }
    }
    send(thisSocket, buf, strlen(buf) + 1, 0);
}

//Send error a error message
void sendError(int errId, int socket){
    char buf[BUFSIZ];
    int cx = snprintf(buf, BUFSIZ, "07 ");
    switch (errId)
    {
    case 1:
        snprintf(buf + cx, BUFSIZ - cx, "01");
        send(socket, buf, strlen(buf) + 1, 0);
        break;
    case 2:
        snprintf(buf + cx, BUFSIZ - cx, "02");
        send(socket, buf, strlen(buf) + 1, 0);
        break;

    case 3:
        snprintf(buf + cx, BUFSIZ - cx, "03");
        send(socket, buf, strlen(buf) + 1, 0);
        break;
    case 4:
        snprintf(buf + cx, BUFSIZ - cx, "04");
        send(socket, buf, strlen(buf) + 1, 0);
        break;

    default:
        printf("Unknown error.\n");
        break;
    }
}

//Funcao de tratamento de mensagem recebida pelo cliente
int handleMessage_C(char* data, int* equipments, int socket, int* id){
    char* sub;
    char buf[BUFSIZ];
    memset(buf, 0, BUFSIZ);

    if((sub = strtok(data, " "))!= NULL){
        //RES_ADD
        if(strcmp(sub, "03")== 0){
            sub = strtok(NULL, " ");
            if(sub == NULL){return 0;}
            char* isNew = sub;
            isNew = strtok(NULL, "_");
            if(isNew == NULL){
                int newId = atoi(sub);
                equipments[newId - 1] = 1;
                printf("Equipment %d added\n", newId);
            }else{
                *id = atoi(isNew);
                equipments[*id - 1] = 1;
                printf("New Id: %d\n", *id);  
            }
        }
        //RES_LIST
        else if(strcmp(sub, "04")== 0){
            sub = strtok(NULL, " ");
            if(sub == NULL){return 0;}
            while(sub != NULL){
                int newId = atoi(sub);
                equipments[newId - 1] = 1;
                sub = strtok(NULL, " ");
            }
        }
        //REQ_INF
        else if(strcmp(sub, "05")== 0){
            sub = strtok(NULL, " ");
            if(sub == NULL){return 0;}
            int originId = atoi(sub);
            sub = strtok(NULL, " ");
            if(sub == NULL){return 0;}
            int destId = atoi(sub);
            if(destId != *id){
                printf("Wrong id request.\n");
            }
            else{
                printf("requested information\n");
                float val = genRandValue();
                snprintf(buf, BUFSIZ, "06 %d %d %.2f", destId, originId, val);
                send(socket, buf, strlen(buf) + 1, 0);
            }
        }
        //RES_INF
        else if(strcmp(sub, "06")== 0){
            sub = strtok(NULL, " ");
            if(sub == NULL){return 0;}
            int originId = atoi(sub);
            sub = strtok(NULL, " ");
            if(sub == NULL){return 0;}
            int destId = atoi(sub);
            if(destId != *id){
                printf("Wrong id request.\n");
            }
            else{
                sub = strtok(NULL, " ");
                if(sub == NULL){return 0;}
                float val = atof(sub);
                printf("Value from %d : %f\n", originId, val);
            }
        }
        else if(strcmp(sub, "07")== 0){
            sub = strtok(NULL, " ");
            if(sub == NULL){return 0;}
            int errId = atoi(sub);
            handleError(errId);
        }
        else if(strcmp(sub, "08")== 0){
            printf("Success\n");
            return 1;
        }
    }
    return 0;
}

//Returns a two places decimal random number between 0 and 10
float genRandValue(){
    float val = ((float)rand()/(float)(RAND_MAX)) * 10;
    val = (val * 100) / 100;
    return val;
}

//Handle received error messages
void handleError(int errId){
    switch (errId)
    {
    case 1:
        printf("Equipment not found\n");
        break;

    case 2:
        printf("Source equipment not found\n");
        break;

    case 3:
        printf("Target equipment not found\n");
        break;

    case 4:
        printf("Equipment limit exceeded");
        break;
        
    default:
        printf("Unknown error\n");
        break;
    }
}

//Le o comando do cliente e, se necessario, envia ao servidor
int readInput(int socket, int id, int* equip){
    char buf[BUFSIZ];
    memset(buf, 0, BUFSIZ);
    fgets(buf, BUFSIZ, stdin);
    
    if(strcmp(buf, "close connection\n") == 0){
        memset(buf, 0, BUFSIZ);
        snprintf(buf, sizeof(buf), "02 %d", id);
        send(socket, buf, strlen(buf) + 1, 0);
        close(socket);
        exit(EXIT_SUCCESS);
    }
    else if(strcmp(buf, "list equipment\n") == 0){
        memset(buf, 0, BUFSIZ);
        snprintf(buf, sizeof(buf), "09");
        send(socket, buf, strlen(buf) + 1, 0);
        listMyEquips(equip);
    }
    else if(strstr(buf, "request information from") != NULL){
        char* sub = strtok(buf, " ");
        for(int i = 0; i<3; i++){
            sub = strtok(NULL, " ");
        }
        int reqId = atoi(sub);
        memset(buf, 0, BUFSIZ);
        snprintf(buf, sizeof(buf), "05 %d %d", id, reqId);
        send(socket, buf, strlen(buf) + 1, 0);
    }
    else{
        printf("Unknown command\n");
        return 1;
    }
    return 0;
}

//List equipment database
void listMyEquips(int* equips){
    for(int i = 0; i<10; i++){
        if(equips[i]){
            printf("%d ", i+1);
        }
    }
    printf("\n");
}