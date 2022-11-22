#pragma once

#include <stdlib.h>
#include <stdio.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <string.h>
#include <unistd.h>

void logExit(const char *msg);

int addrParse(const char *addrStr, const char *portStr, struct sockaddr_storage *storage);

void addrToStr(const struct sockaddr *addr, char *str, size_t strSize);

int ServerSockInit(const char *portStr,
                   struct sockaddr_storage *storage);

int handleMessage_S(char* data, int* equipaments, int* csockets, int thisSocket);

int addEquip(int* equipaments, int* csockets, int thisSocket);

int removeEquip(int id, int* equipaments, int* csockets);

void sendToAll(char* buf, int* equipments,int* csockets, int id);

void listEquips(int* equipaments, int thisSocket);

void sendError(int errId, int socket);

int handleMessage_C(char* data, int* equipments, int socket, int* id);

float genRandValue();

void handleError(int errId);

int readInput(int socket, int id, int* equip);

void listMyEquips(int* equips);