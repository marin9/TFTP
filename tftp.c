#include "tftp.h"
#include "file.h"
#include <arpa/inet.h>
#include <string.h>

char *TMPFILE="/tmp/tftp.tmp";

int GetRequestData(char *buff, int len, int *opcode, char *filename){
	//get opcode
	*opcode=ntohs(*((short*)buff));
	if(*opcode!=HELLO && *opcode!=READ && *opcode!=WRITE && *opcode!=LDIR && *opcode!=REMOVE){
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
	if(strstr(filename, "..")!=NULL){
		return ACCESS_VIOLATION;
	}
	
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
	if(!wr){
		SendError(sock, buff, ACCESS_VIOLATION, "Access violation.", addr, alen);
		return;
	}
	
	strcpy(buff, dir);
	strcat(buff, "/");
	strcat(buff, name);
	
	if(strstr(buff, "..")!=NULL){
		SendError(sock, buff, ACCESS_VIOLATION, "", addr, alen);
		return;
	}
	
	printf("%s\n", buff);
	int ret=remove(buff); 
	
	if(ret==-1) SendError(sock, buff, NOT_DEFINED, strerror(errno), addr, alen);
	else SendAck(sock, buff, 0, addr, alen);
}

void WriteFile(int sock, char *buff, char *dir, char *name, int wr, struct sockaddr_in *addr, socklen_t alen){
	if(!wr){
		SendError(sock, buff, ACCESS_VIOLATION, "Access violation.", addr, alen);
		return;
	}
	
	strcpy(buff, dir);
	strcat(buff, "/");
	strcat(buff, name);
	
	FILE *file=fopen(buff, "r");
	if(file!=NULL){
		SendError(sock, buff, FILE_ALREADY_EXIST, "File already exist.", addr, alen);
		return;
	}
		
	file=fopen(buff, "w+");
	if(file==NULL){
		if(errno==EACCES) SendError(sock, buff, ACCESS_VIOLATION, "Access violation.", addr, alen);
		else SendError(sock, buff, 0, strerror(errno), addr, alen);
		return;
	}
	
	int n;
	int ok=0;
	int packNum=0;
	struct sockaddr_in recvAddr;
	socklen_t len=sizeof(recvAddr);
	
	SendAck(sock, buff, 0, addr, alen);
	++packNum;
	
	int next=1;
	while(next){
		int i;
		for(i=0;i<5;++i){
			n=recvfrom(sock, buff, BUFFLEN, 0, (struct sockaddr*)&recvAddr, &len);			
			if(n==-1 && (errno==EAGAIN || errno==EWOULDBLOCK)) continue;
			if(!equals(addr, &recvAddr)){
				SendError(sock, buff, UNKNOWN_PORT, "Who are you ?", &recvAddr, len);
				continue;
			}
			
			if(ntohs(*((unsigned short*)buff))!=DATA) continue;		
			if(ntohs(*((unsigned short*)(buff+2)))==packNum) break;
		}
			
		if(i==5) break;	
							
		int stat=fwrite(buff+4, 1, n-4, file);	
		if(stat!=(n-4)){
			//ENOMEM or EDQUOTA
			SendError(sock, buff, NOT_DEFINED, "Error on write in file.", &recvAddr, len);
			break;
		}			
			
		*((short*)buff)=htons(ACK);
		*((short*)(buff+2))=htons(packNum);	
		SendAck(sock, buff, packNum, addr, alen);
			
		if(n<BUFFLEN){
			ok=1;
			break;
		}
		++packNum;
	}	
	fclose(file);
	
	if(!ok) RemoveFile(sock, buff, dir, name, 1, addr, alen);
}

void SendFile(int sock, char *buff, char *dir, char *name, struct sockaddr_in *addr, socklen_t alen){
	if(dir==NULL || name==NULL){
		strcpy(buff, TMPFILE);
	}else{
		strcpy(buff, dir);
		strcat(buff, "/");
		strcat(buff, name);
	}		
	
	FILE *file=fopen(buff, "r");
	if(file==NULL){
		if(errno==ENOENT) SendError(sock, buff, FILE_NOT_FOUND, "File not found.", addr, alen);
		else if(errno==EACCES) SendError(sock, buff, ACCESS_VIOLATION, "Access violation.", addr, alen);
		else SendError(sock, buff, 0, strerror(errno), addr, alen);
		return;
	}
	
	int n;
	int packNum=0;
	struct sockaddr_in recvAddr;
	socklen_t len=sizeof(recvAddr);
	char recvBuff[BUFFLEN];
	
	while(!feof(file)){
		++packNum;
		*((short*)buff)=htons(DATA);
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
	strcpy(buff, dir);
	strcat(buff, "/");
	strcat(buff, name);
		
	if(!dirExist(buff)){
		SendError(sock, buff, DIRECTORY_NOT_FOUND, "Directory not found.", addr, alen);
		return;
	}
	
	File *list=listFiles(buff);		
	File *flist=list;
	
	char *data=(char*)malloc(512*sizeof(char));
	int size=512;
	data[0]=0;
	
	while(flist!=NULL){	
		int nlen=strlen(flist->name);
		int slen;
		char file_size[16];
		if(flist->isDir){
			strcpy(file_size, "DIR");
			slen=strlen(file_size);
		}else{
			sizeToH(flist->size, file_size, 16);
			slen=strlen(file_size);
		}
		
		if(size<=(nlen+slen+1)){
			size=size+512;
			data=(char*)realloc(data, size*sizeof(char));
		}
				
		strcat(data, flist->name);
		strcat(data, "\t\t");
		strcat(data, file_size);
		strcat(data, "\n");
		
		flist=flist->next;
	}
	
	FILE *f=fopen(TMPFILE, "w+");
	fwrite(data, strlen(data), 1, f);
	fclose(f);
	
	SendFile(sock, buff, NULL, NULL, addr, alen);

	freeFiles(list);	
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

void SendHello(int sock, char *buff, struct sockaddr_in *addr, socklen_t len){
	char hostname[128];
	gethostname(hostname, 128);
	
	*((short*)buff)=htons(HELLO);	
	strcpy(buff+4, hostname);
	
	sendto(sock, buff, 4+strlen(buff+4)+1, 0, (struct sockaddr*)addr, len);		
}

