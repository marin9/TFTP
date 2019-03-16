#pragma once
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
int SocketUDP(unsigned short port);
int equals(struct sockaddr_in* addr1, struct sockaddr_in* addr2);

