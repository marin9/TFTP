#include "net.h"


void printLocalAddrs(){
	char hostname[128];
    gethostname(hostname, 128);
	printf("Host name: %s\n\n", hostname);

	struct ifaddrs *addr, *paddr;
    if(getifaddrs(&addr)==-1){
		printf("\x1B[33mWARNING:\x1B[0m Unknown local address.\n");
    }
	paddr=addr;
	
    char host[NI_MAXHOST];

	printf("Interfaces:\nAddress \t Netmask\n");
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

char* getIPv4ByName(char *name){
	struct addrinfo hints, *res, *pom;

	memset(&hints, 0, sizeof(hints));
	hints.ai_family=AF_INET;
	hints.ai_flags |= AI_CANONNAME;

	if(getaddrinfo(name, NULL, &hints, &res)) return NULL;

	pom=res;
	char *address=(char*)malloc(NI_MAXHOST);	
	
	inet_ntop(res->ai_family, &((struct sockaddr_in*)res->ai_addr)->sin_addr,address,NI_MAXHOST);

	freeaddrinfo(pom);
	return address;
}

char* getNameByIP(char *ip_str){
	char *name=(char*)malloc(NI_MAXHOST);

	struct sockaddr_in sa;
	sa.sin_family=AF_INET;

	if(inet_pton(AF_INET, ip_str, &(sa.sin_addr))!=1){
		free(name);
		return NULL;
	}

	if(getnameinfo((struct sockaddr*)&sa,sizeof(struct sockaddr_in), name, NI_MAXHOST, NULL, 0,NI_NAMEREQD)!=0){
		free(name);
		return NULL;
	}
	return name;	
}


int SocketUDP(unsigned short port){
	int sock;
	struct sockaddr_in addr;

	if((sock=socket(PF_INET, SOCK_DGRAM, 0))==-1){
		printf("\x1B[31mERROR:\x1B[0m UDP socket bind fail: %s.\n", strerror(errno));
		exit(2);
	}
	
	addr.sin_family=AF_INET;
	addr.sin_port=htons(port);
	addr.sin_addr.s_addr=INADDR_ANY;

	if(bind(sock, (struct sockaddr*)&addr, sizeof(addr))<0){
		printf("\x1B[31mERROR:\x1B[0m UDP socket bind fail: %s.\n", strerror(errno));
		exit(2);
	}

	return sock;
}



