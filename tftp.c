#include "tftp.h"
#include <arpa/inet.h>
#include <string.h>

//TODO
//tftp.c
//ctftp.c


int GetRequestData(char *buff, int len, int *opcode, char *filename){
	//get opcode
	*opcode=ntohs(*((short*)buff));
	if(*opcode!=HELLO && *opcode!=READ && *opcode!=WRITE && *opcode!=DIR && *opcode!=REMOVE){
		return ILLEGAL_TFTP_OPERATION;
	}
	
	//get filename
	int i, j;
	for(i=2, j=0;i<len || buff[i]!='\0';++i, ++j){
		filename[j]=buff[i];
	}
	if(i>=BUFFLEN) return ILLEGAL_TFTP_OPERATION;
	filename[j]=0;
	++i;
	
	//get transfer type
	char transfertype[32];
	for(j=0;i<len || buff[i]!='\0' || j>=32;++i){
		transfertype[j]=buff[i];
	}
	if(i>=BUFFLEN) return ILLEGAL_TFTP_OPERATION;
	transfertype[j]=0;
	
	if(strcmp(transfertype, "octet")!=0 && strcmp(transfertype, "OCTET")!=0) return ILLEGAL_TFTP_OPERATION;
	return 0;
}

void RemoveFile(int sock, char *buff, char *dir, char *name, int wr, struct sockaddr_in *addr, socklen_t alen){
	
	
	
	
	
	
	
	
}

void WriteFile(int sock, char *buff, char *dir, char *name, int wr, struct sockaddr_in *addr, socklen_t alen){










}

void SendFile(int sock, char *buff, char *dir, char *name, struct sockaddr_in *addr, socklen_t alen){
	char path[BUFFLEN-5];
	strcpy(path, dir);
	strcat(path, "/");
	strcat(path, name);
	
	FILE *file=fopen(path, "r");
	if(file==NULL){
		if(errno==ENOENT) SendError(sock, buff, 1, "File not found.", addr, alen);
		else if(errno==EACCESS) SendError(sock, buff, 2, "Access violation.", addr, alen);
		else SendError(sock, buff, 0, strerror(errno), addr, alen);
	}
	
	
	int n;
	do{
		*((short*)buff)=htons(3);
		*((short*)(buff+2))=htons(1);
		n=fread(buff+4, 1, BUFFLEN-4, file);
		
		if(n==0 && );
		
		
		sendto();
	}
	//#define UNKNOWN_PORT			5
}

void SendDir(int sock, char *buff, char *dir, char *name, struct sockaddr_in *addr, socklen_t alen){
	
	
	
	
	
	
	
	
	
}

void SendError(int sock, char *buff, int error, char *msg, struct sockaddr_in* addr, socklen_t len){
	*((short*)buff)=htons(ERROR);
	*((short*)(buff+2))=htons(error);	
	strcpy(buff+4, msg);
	
	sendto(sock, buff, 4+strlen(buff+4)+1, 0, (struct sockaddr*)addr, len);	
}

void SendAck(int sock, char* buff, int num, struct sockaddr_in *addr, socklen_t len){
	*((short*)buff)=htons(ACK);
	*((unsigned short*)(buff+2))=htons(num);
	
	sendto(sock, buff, 4, 0, (struct sockaddr*)addr, len);
}


