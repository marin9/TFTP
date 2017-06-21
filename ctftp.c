#include <stdio.h>

#include "net.h"
#include "tftp.h"
//TODO
//test

//ctftp.c

void GetOpt(int argc, char **argv, unsigned short *port);
void PrepareAddrBroadcast(int socket, struct sockaddr_in *addr, unsigned short port);
void GetServerAddr(int sock, char *buff, struct sockaddr_in *addr, char *hostname);
void Send(int sock, char* buff, struct sockaddr_in *addr);


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
	
	
	printf("Finish:%s\n", hostname);
	
	
	return 0;
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
	inet_pton(AF_INET, "127.0.0.1", &(addr->sin_addr)); //TODO  255.255.255.255

	int on=1;
	if(setsockopt(socket, SOL_SOCKET, SO_BROADCAST, &on, sizeof(on))<0){
		printf("\x1B[31mERROR:\x1B[0m Set socket broadcast option fail.\n");
		exit(2);
	}
}

void GetServerAddr(int sock, char *buff, struct sockaddr_in *addr, char *hostname){
	socklen_t addrlen=sizeof(*addr);

	*((unsigned short*)buff)=HELLO;
	
	while(1){//TODO get host name
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
	printf("\x1B[32mServer control ready.\x1B[0m \nHost name: %s\n", buff);
}

void Send(int sock, char* buff, struct sockaddr_in *addr){
	int msglen=strlen(buff);
	if(sendto(sock, buff, msglen+1, 0, (struct sockaddr*)addr, sizeof(*addr))!=(msglen+1)){
		printf("\x1B[31mERROR:\x1B[0m Send fail: %s\n", strerror(errno));
		exit(3);
	}
}

