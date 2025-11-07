//Iterative Chat

#include<sys/types.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<arpa/inet.h>
#include<string.h>
#include<stdlib.h>
#include<stdio.h>
#include<unistd.h>
struct sockaddr_in serv_addr;

int skfd, r, w;

unsigned short serv_port = 25020; //port number used by the server
char serv_ip[]="192.168.21.25";//server IP Address

//char rbuff[128]; //buffer for recieving messages
// buffer for sending messages


int main()
{
	//initialising server socket address structure with zero values
	bzero(&serv_addr,sizeof(serv_addr));
	
	//filling up the server socket address structure with appropriate values
	
	serv_addr.sin_family = AF_INET;// address family
	serv_addr.sin_port = htons(serv_port);// port number
	inet_aton(serv_ip, (&serv_addr.sin_addr));// IP address
	
	printf("\nTCP ECHO CLIENT\n");
	
	//creating socket
	if((skfd = socket(AF_INET, SOCK_STREAM, 0))<0)
	{
		printf("\nCLIENT ERROR: Cannot create socket.\n");
		exit(1);
	}
	
	//request server for a connection
	if((connect(skfd, (struct sockaddr*)&serv_addr, sizeof(serv_addr)))<0)
	{
		printf("\nCLIENT ERROR: Cannot connect to the server.\n");
		close(skfd);
		exit(1);
	}
	printf("\nCLIENT: Connected to the server.\n");
	//For Iterative Chat
	for(;;) 
      	{
      		char sbuff[128];
      		//scans the client's message
		printf("Enter Client's message:\n");
		//scanf("%s",sbuff);
		fgets(sbuff, sizeof(sbuff), stdin);
		printf("\nCLIENT: Message to the server: %s\n",sbuff);
		//Sending message to server
      		if((w=write(skfd,sbuff,128))<0)
		{
		printf("\nCLIENT ERROR: Cannot send message to the server\n");
		close(skfd);
		exit(1);
		}
		printf("\nCLIENT: Message sent to server.\n");
		
		//Stop the connection if the client sends "stop"
		
		if (strcmp(sbuff, "STOP") == 0) {
        		printf("Stopping the connection...");
           		exit(1); // Exit the loop
     	   	}
     	   	
        	// Reading message from Server
        	if((r=read(skfd,sbuff,128))<0)
			printf("\nCLIENT SERVER: Cannot recieve message from the server\n");
		else
		{
			sbuff[r]='\0';
			//print the recieved message on console
			printf("\nCLIENT: Message from the server: %s\n",sbuff);
		}
	
       	 //Stop the connection if the server sends "stop"
        	if (strcmp(sbuff,"STOP") == 0) {
        		printf("Stopping the connection...");
           		break; // Exit the loop
     	   	}
  	  
            }
        close(skfd);
        return 0;
}
