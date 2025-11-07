#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

struct sockaddr_in serv_addr1, sender_addr;

int skfd, r, bind1;
unsigned short serv_port = 25021;
char rbuff[128];
socklen_t sender_len;

int main()
{
	bzero(&serv_addr1, sizeof(serv_addr1));
	serv_addr1.sin_family = AF_INET;
	serv_addr1.sin_port = htons(serv_port);
	serv_addr1.sin_addr.s_addr = htonl(INADDR_ANY); // Listen on all interfaces
	
	// Create a UDP socket
	skfd = socket(AF_INET,SOCK_DGRAM,0);
	if(skfd<0)
	{
		perror("Error in socket creation\n");
		exit(1);
	}
	int reuse = 1;
	if(setsockopt(skfd,SOL_SOCKET,SO_REUSEADDR,(char*)&reuse,sizeof(reuse))<0)
	{
		perror("setsockopt (SO_REUSEADDR) failed");
		close(skfd);
		exit(1);
	}
	
	bind1 = bind(skfd,(struct sockaddr*)&serv_addr1,sizeof(serv_addr1));
	if(bind1<0)
	{
		perror("Binding error");
		close(skfd);
		exit(1);
	}
	
	printf("UDP BROADCASTING CLIENT WAITING ON PORT %d...\n",serv_port);
	
	// Receive broadcasting messages indefinitely
	sender_len = sizeof(sender_addr);
	for(;;)
	{
		r = recvfrom(skfd,rbuff,sizeof(rbuff)-1,0,(struct sockaddr*)&sender_addr,&sender_len);
		if(r<0)
		{
			perror("Receive error\n");
			continue;
		}
		
		rbuff[r] = '\0'; // Null terminate the received message
		
		printf("\nMessage received from %s:%d ->'%s\n",inet_ntoa(sender_addr.sin_addr),ntohs(sender_addr.sin_port),rbuff);
	}
	
	close(skfd);
	return 0;
}
