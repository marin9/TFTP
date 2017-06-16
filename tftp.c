#include "tftp.h"
#include <arpa/inet.h>
#include <string.h>


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
	strcpy(buff, dir);
	strcat(buff, "/");
	strcat(buff, name);
	
	if(strstr(buff, "..")!=NULL){
		SendError(sock, buff, ACCESS_VIOLATION, "", addr, alen);
		return;
	}
	
	printf("%s\n", buff);
	int ret=1; //remove(buff);  //TODO test
	
	if(ret==-1) SendError(sock, buff, NOT_DEFINED, strerror(errno), addr, alen);
	else SendAck(sock, buff, 0, addr, alen);
}

void WriteFile(int sock, char *buff, char *dir, char *name, int wr, struct sockaddr_in *addr, socklen_t alen){










}

void SendFile(int sock, char *buff, char *dir, char *name, struct sockaddr_in *addr, socklen_t alen){
	strcpy(buff, dir);
	strcat(buff, "/");
	strcat(buff, name);
	
	FILE *file=fopen(buff, "r");
	if(file==NULL){
		if(errno==ENOENT) SendError(sock, buff, 1, "File not found.", addr, alen);
		else if(errno==EACCES) SendError(sock, buff, 2, "Access violation.", addr, alen);
		else SendError(sock, buff, 0, strerror(errno), addr, alen);
	}
	
	int n;
	int packNum=0;
	struct sockaddr_in recvAddr;
	socklen_t len=sizeof(recvAddr);
	char recvBuff[BUFFLEN];
	
	while(!feof(file)){
		++packNum;
		*((short*)buff)=htons(3);
		*((short*)(buff+2))=htons(packNum);
		
		n=fread(buff+4, 1, DATALEN, file);				
			
		int i;
		int sendAgain=1;
		for(i=0;i<5;++i){	
			if(sendAgain) sendto(sock, buff, n+4, 0, (struct sockaddr*)addr, alen);		
			n=recvfrom(sock, recvBuff, BUFFLEN, 0, (struct sockaddr*)&recvAddr, &len);
			sendAgain=1;
			
			if(n==-1 && (errno==EAGAIN || errno==EWOULDBLOCK)) continue;
			
			if(!equals(addr, &recvAddr)){
				SendError(sock, buff, UNKNOWN_PORT, "Who are you ?", &recvAddr, len);
				continue;
			}
			
			if(ntohs(*((unsigned short*)recvBuff))!=ACK) continue;
			
			if(ntohs(*((unsigned short*)(recvBuff+2)))==packNum) break;
			
			if(ntohs(*((unsigned short*)(recvBuff+2)))==(packNum-1)){
				sendAgain=0;
			}			
		}		
		if(i==5) break;
	}	
	fclose(file);
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
