#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

struct sockaddr_in serv_addr;
int listenfd,w,BP; //Broadcast Prermission

unsigned short serv_port = 25021;
char serv_ip[] = "255.255.255.255"; //Broadcast address
char rbuff[128] = "Hello, this is Deb's broadcast message";

int main()
{
	BP = 1;// Enable flag
	
	// Create udp socket
	listenfd = socket(AF_INET, SOCK_DGRAM,0);
	if(listenfd<0)
	{
		perror("Socket creation failed\n");
		exit(1);
	}
	
	// Enable broadcast permission
	if(setsockopt(listenfd, SOL_SOCKET,SO_BROADCAST,(void*)&BP,sizeof(BP))<0)
	{
		perror("Error enabling broadcast option\n");
		close(listenfd);
		exit(1);
	}
	
	// Initialize server address structure
	bzero(&serv_addr,sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_port = htons(serv_port);
	serv_addr.sin_addr.s_addr = inet_addr(serv_ip); // Use specific broadcast ip
	
	printf("UDP BROADCAST SERVER STARTED on %s:%d\n",serv_ip,serv_port);
	
	while(1)
	{
		w = sendto(listenfd,rbuff,strlen(rbuff)+1,0,(struct sockaddr*)&serv_addr,sizeof(serv_addr));
		if(w<0)
			perror("Cannot send message\n");
		else
			printf("Broadcasted: '%s'\n",rbuff);
			
		sleep(3); //  Sleep 3 seconds before next broadcast
	}
	
	// Unreachable, but good practice
	close(listenfd);
	return 0;
}
