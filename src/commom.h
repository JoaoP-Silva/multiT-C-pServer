#pragma once

#include <stdlib.h>
#include <stdio.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>

void logExit(const char *msg);

int addrParse(const char *addrStr, const char *portStr, struct sockaddr_storage *storage);

void addrToStr(const struct sockaddr *addr, char *str, size_t strSize);

int ServerSockInit(const char *proto, const char *portStr,
                   struct sockaddr_storage *storage);

void handleMessage_S(char* data, int* equipaments, int* csockets, int thisSocket);

int addEquip(int* equipaments, int* csockets, int thisSocket);

int removeEquip(int id, int* equipaments, int* csockets);

void sendToAll(char[] buf, int* csockets);

void listEquips(int* equipaments, int thisSocket)

void handleMessage_C(char* data, int* equipaments);