#include "net.h"


void printLocalAddrs(){
	char hostname[128];
    gethostname(hostname, 128);
	printf(" Host name: %s\n\n", hostname);

	struct ifaddrs *addr, *paddr;
    if(getifaddrs(&addr)==-1){
		printf("\x1B[33m WARNING:\x1B[0m Unknown local address.\n");
    }
	paddr=addr;
	
    char host[NI_MAXHOST];

	printf(" Interfaces:\nAddress \t Netmask\n");
    for(;addr!=NULL;addr=addr->ifa_next){
        if(addr->ifa_addr->sa_family==AF_INET){
            getnameinfo(addr->ifa_addr, sizeof(struct sockaddr_in), host, NI_MAXHOST, NULL, 0, NI_NUMERICHOST);
            printf("%s\t", host);
			getnameinfo(addr->ifa_netmask, sizeof(struct sockaddr_in), host, NI_MAXHOST, NULL, 0, NI_NUMERICHOST);
            printf("%s\n", host);
        }
    }
	printf("\n");
	freeifaddrs(paddr);
}

int SocketUDP(unsigned short port){
	int sock;
	struct sockaddr_in addr;

	if((sock=socket(PF_INET, SOCK_DGRAM, 0))==-1){
		printf("\x1B[31m ERROR:\x1B[0m UDP socket bind fail: %s.\n", strerror(errno));
		exit(2);
	}
	
	int on=1;
	if(setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on))<0){
		printf("\x1B[31m ERROR:\x1B[0m UDP set socket option fail: %s.\n", strerror(errno));
		exit(2);
	}
	
	struct timeval time;
	time.tv_sec=1;
	time.tv_usec=0;
	if(setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, &time, sizeof(time))<0){
		printf("\x1B[31m ERROR:\x1B[0m UDP set socket recv timeout fail: %s.\n", strerror(errno));
		exit(2);
	}
	
	addr.sin_family=AF_INET;
	addr.sin_port=htons(port);
	addr.sin_addr.s_addr=INADDR_ANY;

	if(bind(sock, (struct sockaddr*)&addr, sizeof(addr))<0){
		printf("\x1B[31m ERROR:\x1B[0m UDP socket bind fail: %s.\n", strerror(errno));
		exit(2);
	}

	return sock;
}

int equals(struct sockaddr_in* addr1, struct sockaddr_in* addr2){
	unsigned short port1=addr1->sin_port;
	unsigned short port2=addr2->sin_port;
	
	if(port1!=port2) return 0;
	
	char host1[NI_MAXHOST];
	char host2[NI_MAXHOST];	
	inet_ntop(AF_INET, &(addr1->sin_addr), host1, INET_ADDRSTRLEN);
	inet_ntop(AF_INET, &(addr2->sin_addr), host2, INET_ADDRSTRLEN);
	
	if(strcmp(host1, host2)!=0) return 0;
	
	return 1;
}

