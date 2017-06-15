#ifndef NET_H
#define NET_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <ifaddrs.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <errno.h>


void printLocalAddrs();
char* getIPv4ByName(char *name);
char* getNameByIP(char *ip_str);

int SocketUDP(unsigned short port);

#endif

