#include <stdio.h>
#include "net.h"
#include "tftp.h"
//TODO test

void GetOpt(int argc, char **argv, unsigned short *port);
void PrepareAddrBroadcast(int socket, struct sockaddr_in *addr, unsigned short port);
void GetServerAddr(int sock, char *buff, struct sockaddr_in *addr, char *hostname);
void Send(int sock, char* buff, struct sockaddr_in *addr);
void ToLower(char *str, int len);
void GetFile(int sock, char *name, struct sockaddr_in *addr, socklen_t len);
void PutFile(int sock, char *name, struct sockaddr_in *addr, socklen_t len);
void RmvFile(int sock, char *name, struct sockaddr_in *addr, socklen_t len);
void ListFile(int sock, char *name, struct sockaddr_in *addr, socklen_t len);


int main(int argc, char **argv){
	int sock;
	struct sockaddr_in addr;
	socklen_t alen=sizeof(addr);
	unsigned short port;	
	char buffer[BUFFLEN];
	
	GetOpt(argc, argv, &port);
	
	sock=SocketUDP(0);
	
	char hostname[128];
	PrepareAddrBroadcast(sock, &addr, port);		
	GetServerAddr(sock, buffer, &addr, hostname); 
	addr.sin_port=htons(port);
	
	int i;
	while(1){
		printf(">");
		scanf("%s", buffer);
				
		for(i=0;buffer[i]!=':';++i);		
		ToLower(buffer, i);
		
		if(strncmp(buffer, "get", i)==0) GetFile(sock, buffer+i+1, &addr, alen);
		else if(strncmp(buffer, "put", i)==0) PutFile(sock, buffer+i+1, &addr, alen);
		else if(strncmp(buffer, "rmv", i)==0) RmvFile(sock, buffer+i+1, &addr, alen);
		else if(strncmp(buffer, "list", i)==0) ListFile(sock, buffer+i+1, &addr, alen);
		else if(strncmp(buffer, "quit", i)==0) break;
		else if(strncmp(buffer, "help", i)==0){
			printf("Commands:\n");
			printf("get - get file\n");
			printf("put - write file\n");
			printf("rmv - remove file\n");
			printf("list - list directory\n");
			printf("Example: put:filename.ext\n");
						
		}else printf("Illegal command. Write help-for more info.\n");
	}
	return 0;
}


void GetFile(int sock, char *name, struct sockaddr_in *addr, socklen_t len){
	char buffer[BUFFLEN];
	
	*((unsigned short*)buffer)=htons(READ);
	strcpy(buffer+2, name);
	strcpy(buffer+2+strlen(name)+1, "octet");
	
	int n=sendto(sock, buffer, BUFFLEN, 0, (struct sockaddr*)addr, len);
	if(n<0){
		printf("\x1B[31mERROR:\x1B[0m Send request fail: %s.\n", strerror(errno));
		return;
	}
	
	FILE *f=fopen(name, "w+");
	if(f==NULL){
		printf("\x1B[31mERROR:\x1B[0m File create error: %s.", strerror(errno));
		return;
	}
	
	int x=0;
	int err=0;
	int packNum=1;
	struct sockaddr_in saddr;
	socklen_t slen=sizeof(saddr);
	while(1){
		n=recvfrom(sock, buffer, BUFFLEN, 0, (struct sockaddr*)&saddr, &slen);
		if(n<1){
			printf(".\n");
			++x;
			if(x==9){
				printf("Error: no response from server.\n");
				err=1;
				break;
			}else continue;
		}
	
		if(ntohs(*((unsigned short*)buffer))==ERROR){
			printf("Warning: %s\n", buffer+4);
			err=1;
			break;
		}

		if(ntohs(*((unsigned short*)buffer))!=DATA) continue;
		if(ntohs(*((unsigned short*)(buffer+2)))!=packNum) continue;
		
		int stat=fwrite(buffer+4, 1, n-4, f);
		if(stat!=(n-4)){
			printf("\x1B[31mERROR:\x1B[0m File write error: %s.", strerror(errno));
			err=1;
			break;
		}
			
		*((unsigned short*)buffer)=htons(ACK);
		*((unsigned short*)(buffer+2))=htons(packNum);
		
		stat=sendto(sock, buffer, 4, 0, (struct sockaddr*)&saddr, slen);
		if(stat<0){
			printf("\x1B[31mERROR:\x1B[0m Send ack fail: %s.", strerror(errno));
			break;
		}	
		
		if((n-4)<DATALEN){
			printf("Finish.\n");
			break;
		}
		
		++packNum;	
		x=0;	
	}	
	fclose(f);
	if(err) remove(name);
}

void PutFile(int sock, char *name, struct sockaddr_in *addr, socklen_t len){
	//TODO
}

void RmvFile(int sock, char *name, struct sockaddr_in *addr, socklen_t len){
	char buffer[BUFFLEN];
	
	*((unsigned short*)buffer)=htons(REMOVE);
	strcpy(buffer+2, name);
	
	int n=sendto(sock, buffer, BUFFLEN, 0, (struct sockaddr*)addr, len);
	if(n<0){
		printf("\x1B[31mERROR:\x1B[0m Send request fail: %s.\n", strerror(errno));
		return;
	}
	
	int x=0;
	struct sockaddr_in saddr;
	socklen_t slen=sizeof(saddr);
	while(1){
		n=recvfrom(sock, buffer, BUFFLEN, 0, (struct sockaddr*)&saddr, &slen);
		if(n<1){
			printf(".\n");
			++x;
			if(x==9){
				printf("Error: no response from server.\n");
				break;
			}else continue;
		}
	
		if(ntohs(*((unsigned short*)buffer))==ERROR){
			printf("Warning: %s\n", buffer+4);
			break;
		}

		if(ntohs(*((unsigned short*)buffer))!=ACK) continue;
		if(ntohs(*((unsigned short*)(buffer+2)))!=0) continue;	
		else break;
	}	
}

void ListFile(int sock, char *name, struct sockaddr_in *addr, socklen_t len){
	char buffer[BUFFLEN];
	
	*((unsigned short*)buffer)=htons(LDIR);
	strcpy(buffer+2, name);
	
	int n=sendto(sock, buffer, BUFFLEN, 0, (struct sockaddr*)addr, len);
	if(n<0){
		printf("\x1B[31mERROR:\x1B[0m Send request fail: %s.\n", strerror(errno));
		return;
	}
	
	int x=0;
	int packNum=1;
	struct sockaddr_in saddr;
	socklen_t slen=sizeof(saddr);
	while(1){
		n=recvfrom(sock, buffer, BUFFLEN, 0, (struct sockaddr*)&saddr, &slen);
		if(n<1){
			printf(".\n");
			++x;
			if(x==9){
				printf("Error: no response from server.\n");
				break;
			}else continue;
		}
	
		if(ntohs(*((unsigned short*)buffer))==ERROR){
			printf("Warning: %s\n", buffer+4);
			break;
		}

		if(ntohs(*((unsigned short*)buffer))!=DATA) continue;
		if(ntohs(*((unsigned short*)(buffer+2)))!=packNum) continue;
		
		printf("%s", buffer+4);
			
		*((unsigned short*)buffer)=htons(ACK);
		*((unsigned short*)(buffer+2))=htons(packNum);
		
		int stat=sendto(sock, buffer, 4, 0, (struct sockaddr*)&saddr, slen);
		if(stat<0){
			printf("\x1B[31mERROR:\x1B[0m Send ack fail: %s.", strerror(errno));
			break;
		}	
		
		if((n-4)<DATALEN){
			printf("Finish.\n");
			break;
		}
		
		++packNum;	
		x=0;	
	}	
}


void GetOpt(int argc, char **argv, unsigned short *port){
	if(argc!=1 && argc!=3){
		printf("Usage: [-p tftp_port]\n");
		exit(1);
	}
	
	*port=69;
	
	if(argc==3){
		if(strcmp(argv[1], "-p")==0){
			*port=(unsigned short)atoi(argv[2]);
		}else{
			printf("Usage: [-p tftp_port]\n");
			exit(1);
		}
	}
}

void PrepareAddrBroadcast(int socket, struct sockaddr_in *addr, unsigned short port){
	memset(addr, 0, sizeof(*addr));   
	addr->sin_family=AF_INET;
	addr->sin_port=htons(port);
	inet_pton(AF_INET, "127.0.0.1", &(addr->sin_addr)); //TODO 255.255.255.255

	int on=1;
	if(setsockopt(socket, SOL_SOCKET, SO_BROADCAST, &on, sizeof(on))<0){
		printf("\x1B[31mERROR:\x1B[0m Set socket broadcast option fail: %s.\n", strerror(errno));
		exit(2);
	}
}

void GetServerAddr(int sock, char *buff, struct sockaddr_in *addr, char *hostname){
	socklen_t addrlen=sizeof(*addr);

	*((unsigned short*)buff)=HELLO;
	
	while(1){
		Send(sock, buff, addr); 
		int s=recvfrom(sock, buff, BUFFLEN, 0, (struct sockaddr*)addr, &addrlen);

		if(s==-1){
			if(errno==EAGAIN || errno==EWOULDBLOCK) printf("\x1B[33mTime out...\x1B[0m \n");
			else{
				printf("\x1B[31mERROR:\x1B[0m Receive fail: %s\n", strerror(errno));
				break;
			}
		}else{
			int code=ntohs(*((unsigned short*)buff));
			if(code==HELLO){
				strcpy(hostname, buff+4);				
				break;
			}
		}		
	}
	printf("\x1B[32mServer control ready.\x1B[0m \nHost name: %s\n", buff+4);
}

void Send(int sock, char* buff, struct sockaddr_in *addr){
	int msglen=strlen(buff);
	if(sendto(sock, buff, msglen+1, 0, (struct sockaddr*)addr, sizeof(*addr))!=(msglen+1)){
		printf("\x1B[31mERROR:\x1B[0m Send fail: %s\n", strerror(errno));
		exit(3);
	}
}

void ToLower(char *str, int len){
	int i;
	for(i=0;i<len && str[i]!='\0';++i){
		if(str[i]>='A' && str[i]<='Z') str[i]+=32;
	}
}

