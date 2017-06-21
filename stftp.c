#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <sys/wait.h>
#include "file.h"
#include "net.h"
#include "tftp.h"


void GetOpt(int argc, char **argv, char *dir, int *write, unsigned short *port);
void printServerInfo(int port, char *dir);
int RecvFrom(int sock, char* buff, struct sockaddr_in* addr, socklen_t *addrlen);
void AddSigAction();
void sigchld_handler();


int main(int argc, char **argv){
	int sock;
	int write;
	unsigned short port;
	char directory[NAMELEN];
			
	GetOpt(argc, argv, directory, &write, &port);
	AddSigAction();
	
	sock=SocketUDP(port);

	char buffer[BUFFLEN];
	while(1){
		struct sockaddr_in caddr;	
		socklen_t alen=sizeof(caddr);
		
		int len=RecvFrom(sock, buffer, &caddr, &alen);
		if(len==-1) continue;
	
		int s=fork();
		if(s==-1){
			printf("\x1B[31mERROR:\x1B[0m Client process create fail. %s\n", strerror(errno));
			exit(1);
			
		}else if(s>0){
			continue;
			
		}else{
			int csock=SocketUDP(0);
			
			int error;
			int opcode;
			char filename[BUFFLEN-4];
	
			if(!(error=GetRequestData(buffer, len, &opcode, filename))){
				if(error==ILLEGAL_TFTP_OPERATION) SendError(csock, buffer, error, "Illegal tftp operation, only octet.", &caddr, alen);
				else if(error==ACCESS_VIOLATION) SendError(csock, buffer, error, "Access violation.", &caddr, alen);
				close(csock);		
				break;
			}
			
			if(opcode==HELLO) SendAck(csock, buffer, 0, &caddr, alen);
			else if(opcode==REMOVE) RemoveFile(csock, buffer, directory, filename, write, &caddr, alen);
			else if(opcode==WRITE) WriteFile(csock, buffer, directory, filename, write, &caddr, alen);
			else if(opcode==READ) SendFile(csock, buffer, directory, filename, &caddr, alen);	
			else if(opcode==LDIR) SendDir(csock, buffer, directory, filename, &caddr, alen);
			
			close(csock);
			break;
		}				
	}

	return 0;
}



void sigchld_handler(){
	while(waitpid(-1, NULL, WNOHANG)>0);
}

void AddSigAction(){
	struct sigaction sa;
	sa.sa_handler=sigchld_handler;
	sigemptyset(&sa.sa_mask);
	sa.sa_flags=SA_RESTART;
	
	if(sigaction(SIGCHLD, &sa, NULL)==-1){
		printf("\x1B[33mERROR:\x1B[0m Sigaction error. %s\n", strerror(errno));
		exit(1);
	}
}

int RecvFrom(int sock, char* buff, struct sockaddr_in* addr, socklen_t *addrlen){
	int n=recvfrom(sock, buff, BUFFLEN, 0, (struct sockaddr*)addr, addrlen);
	
	if(n==-1 && (errno!=EAGAIN && errno!=EWOULDBLOCK && errno!=EINTR)){
		printf("\x1B[31mERROR:\x1B[0m Reqest receive fail: %s.\n", strerror(errno));
		exit(5);
	}
	return n;
}


void GetOpt(int argc, char **argv, char *dir, int *write, unsigned short *port){
	int c;
	int sum=0;
	
	dir[0]=0;
	*write=0;
	*port=69;
	while((c=getopt(argc, argv, "d:p:w"))!=-1){
    	switch(c){
     		case 'd':			
				strncpy(dir, optarg, NAMELEN-1);
				++sum;
        		break;
      		case 'p':
       			*port=(unsigned short)atoi(optarg);
				++sum;
       			break;
       		case 'w':
				*write=1;
				break;
      		default:
        		printf("Usage: [-d working_directory] [-p tftp_port] [-w]\n");
				exit(1);
      	}
	}
	
	if(argc!=(2*sum+1+*write)){
		printf("Usage: [-d working_directory] [-p tftp_port]\n");
		exit(1);
	}
	
	if(dir[0]!=0 && !dirExist(dir)){
		printf("\x1B[31mERROR:\x1B[0m Directory not exist.\n");
		exit(1);
	}

	if(dir[0]==0) getcwd(dir, NAMELEN-1);

	printServerInfo(*port, dir);
}

void printServerInfo(int port, char *dir){
	printLocalAddrs();
	printf("TFTP port:      %d\n", port);
	printf("Working directory: %s\n", dir);
}
