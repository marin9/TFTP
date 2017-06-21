#include <stdio.h>
#include "net.h"
#include "tftp.h"
//TODO test

void GetOpt(int argc, char **argv, unsigned short *port);
void PrepareAddrBroadcast(int socket, struct sockaddr_in *addr, unsigned short port);
void GetServerAddr(int sock, char *buff, struct sockaddr_in *addr, char *hostname);
void Send(int sock, char* buff, struct sockaddr_in *addr);
void ToLower(char *str, int len);
void GetFile(int sock, char *buff, struct sockaddr_in *addr, socklen_t len);
void PutFile(int sock, char *buff, struct sockaddr_in *addr, socklen_t len);
void RmvFile(int sock, char *buff, struct sockaddr_in *addr, socklen_t len);
void ListFile(int sock, char *buff, struct sockaddr_in *addr, socklen_t len);


int main(int argc, char **argv){
	int sock;
	struct sockaddr_in addr;
	socklen_t alen=sizeof(addr);
	unsigned short port;
	char hostname[128];
	char buffer[BUFFLEN];
	
	GetOpt(argc, argv, &port);
	
	sock=SocketUDP(0);
	
	PrepareAddrBroadcast(sock, &addr, port);		
	GetServerAddr(sock, buffer, &addr, hostname); 
	
	char command[16];
	while(1){
		printf(">");
		scanf("%s", buffer);
		
		int i=0;
		while(buffer[i]!=':') ++i;		
		strncpy(command, buffer, i);
		ToLower(command, 16);
		//TODO stack smash
		if(strcmp(command, "get")==0) GetFile(sock, buffer, &addr, alen);
		else if(strcmp(command, "put")==0) PutFile(sock, buffer, &addr, alen);
		else if(strcmp(command, "rmv")==0) RmvFile(sock, buffer, &addr, alen);
		else if(strcmp(command, "list")==0) ListFile(sock, buffer, &addr, alen);
		else if(strcmp(command, "help")==0){
			printf("Commands:\n");
			printf("get - get file\n");
			printf("put - write file\n");
			printf("rmv - remove file\n");
			printf("list - list directory\n");
			printf("Example: put:filename.ext\n");
						
		}else if(strcmp(command, "quit")==0) break;
		else printf("Illegal command. Write help-for more info.\n");
	}
	return 0;
}


void GetFile(int sock, char *buff, struct sockaddr_in *addr, socklen_t len){
	//TODO
}

void PutFile(int sock, char *buff, struct sockaddr_in *addr, socklen_t len){
	//TODO
}

void RmvFile(int sock, char *buff, struct sockaddr_in *addr, socklen_t len){
	//TODO
}

void ListFile(int sock, char *buff, struct sockaddr_in *addr, socklen_t len){
	//TODO
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
	inet_pton(AF_INET, "255.255.255.255", &(addr->sin_addr)); 

	int on=1;
	if(setsockopt(socket, SOL_SOCKET, SO_BROADCAST, &on, sizeof(on))<0){
		printf("\x1B[31mERROR:\x1B[0m Set socket broadcast option fail.\n");
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
	for(i=0;i<len;++i){
		if(str[i]>='A' && str[i]<='Z') str[i]+=32;
	}
}

