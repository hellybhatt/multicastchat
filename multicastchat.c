#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<errno.h>
#include<netinet/in.h>
#include<arpa/inet.h>
#include<unistd.h>
#include<netdb.h>

void exitWithError(){
	printf("Please provide valid arguments\n");
	exit(-1);
}
void exitSysWithError(const char *call){
	fprintf(stderr, "Syscall %s failed with errno= %d: %s\n", call, errno,
	 strerror(errno));
	exit(-1);
}

int main(int argc, char** argv){

	int portnumber;
	char *peer;
	struct sockaddr_in ina;
	struct in_addr localInterface;
	struct message_type{
	char opcode;
	short length;
	char text[80];
	}finalmsg;
	struct sockaddr_in cli;
	char buffer[100];
	char stuff[100];
	int option =1;
	if (argc != 5){
		exitWithError();
	}
	for(int i=0; i<argc; i++){
		if(strcmp(argv[i],"-port") == 0)
			portnumber =(int)atol(argv[i+1]);
	}
	for(int i=0; i<argc; i++){
		if(strcmp(argv[i],"-mcip") == 0)
			peer = argv[i+1];
	}
	memset((char *) &ina, 0, sizeof(ina));
	ina.sin_family = AF_INET;
	ina.sin_port = htons(portnumber);
	struct hostent *hp;
	hp = gethostbyname(peer);
	int r = inet_pton(AF_INET,peer, &(ina.sin_addr));
	if (r == 0){
		char *IPBuffer = inet_ntoa(*((struct in_addr*)hp->h_addr_list[0]));
		//printf("Invalid conversion from text to IP address");
		ina.sin_addr.s_addr=inet_addr(IPBuffer);
	}
	else
		ina.sin_addr.s_addr=inet_addr(peer);
	int len;
	len = sizeof(ina);
	int s = socket(AF_INET, SOCK_DGRAM,0);
		if (s==-1){
		printf("socket failed udp\n");
		exitSysWithError("socket()");
		}
          	char loopch = 0;
		if(setsockopt(s, IPPROTO_IP, IP_MULTICAST_LOOP, (char *)&loopch, sizeof(loopch)) < 0)
		{
			printf("Setting IP_MULTICAST_LOOP error\n");
                        exitSysWithError("setsockopt()");
		}
		localInterface.s_addr = htonl(INADDR_ANY);
		if(setsockopt(s, IPPROTO_IP, IP_MULTICAST_IF, &localInterface, sizeof(localInterface)) < 0)
		{
			printf("Setting local interface error\n");
                        exitSysWithError("setsockopt()");
		}
		printf("Enter your name:\n");
        	char name[32];
        	fflush(stdout);
        	fgets(name, sizeof(name), stdin);
        	name[strcspn(name,"\n")] = 0;
        	printf("----Welcome to the chat----\n");
		printf("%s>",name);
		fflush(stdout);
		fgets(buffer, sizeof(buffer), stdin);
		int sendval = sendto(s,buffer, sizeof(buffer),0,(const struct sockaddr *)&ina, len);
		if(sendval < 0)
		{
                        printf("sendto failed\n");
                        exitSysWithError("sendto()");
                }
		printf("the send messsage is:%s",buffer);
		//sending messages
		struct sockaddr_in localSock;
		memset((char *) &localSock, 0, sizeof(localSock));
		localSock.sin_family = AF_INET;
		localSock.sin_port = htons(portnumber);;
		localSock.sin_addr.s_addr  = htonl(INADDR_ANY);
		if (setsockopt(s, SOL_SOCKET, (SO_REUSEPORT|SO_REUSEADDR),
				(char*)&option, sizeof(option)) < 0){
			printf("set sock failed udp\n");
                	exitSysWithError("setsockopt()");
		}
		if ((bind(s,(struct sockaddr *)&localSock, sizeof(localSock))) < 0);
		{
			printf("bindfailed\n");
			exitSysWithError("bind()");
		}
		struct ip_mreq mreq;
		mreq.imr_multiaddr.s_addr = inet_addr(peer);
		mreq.imr_interface.s_addr = htonl(INADDR_ANY);
		if (setsockopt(s, IPPROTO_IP, IP_ADD_MEMBERSHIP, &mreq, sizeof(mreq)) < 0)
		{
                        printf("Adding memebership error\n");
                        exitSysWithError("setsockopt()");
                }
		int n = recv(s,(char *)buffer,100,0);
		if(n < 0)
                {
                        printf("recvfrom failed\n");
                        exitSysWithError("recvfrom()");
                }
		printf("%s",buffer);
}
