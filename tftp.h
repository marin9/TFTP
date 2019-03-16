#pragma once
#include "net.h"

//Request codes
#define HELLO	0
#define READ	1
#define WRITE	2
#define DATA	3
#define ACK		4
#define ERROR	5
#define REMOVE	6
#define LDIR	7

//Error codes
#define NOT_DEFINED				0
#define FILE_NOT_FOUND			1
#define ACCESS_VIOLATION		2
#define DISK_FULL				3
#define ILLEGAL_TFTP_OPERATION	4
#define UNKNOWN_PORT			5
#define FILE_ALREADY_EXIST		6
#define NO_SUCH_USER			7
#define DIRECTORY_NOT_FOUND		8		

#define DATALEN		512
#define BUFFLEN		(2+2+DATALEN)

int GetRequestData(char *buff, int len, int *opcode, char *filename);
void RemoveFile(int sock, char *buff, char *dir, char *name, int wr, struct sockaddr_in *addr, socklen_t alen);
void WriteFile(int sock, char *buff, char *dir, char *name, int wr, struct sockaddr_in *addr, socklen_t alen);
void SendFile(int sock, char *buff, char *dir, char *name, struct sockaddr_in *addr, socklen_t alen);
void SendDir(int sock, char *buff, char *dir, char *name, struct sockaddr_in *addr, socklen_t alen);
void SendError(int sock, char *buff, int error, char *msg, struct sockaddr_in* addr, socklen_t len);
void SendAck(int sock, char* buff, int num, struct sockaddr_in *addr, socklen_t len);
void SendHello(int sock, char *buff, struct sockaddr_in *addr, socklen_t len);

