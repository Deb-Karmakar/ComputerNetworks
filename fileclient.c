//TCP Echo Client Program
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
char serv_ip[]="127.0.0.1";//server IP Address

char rbuff[128]; //buffer for receiving messages
char sbuff[128]; // buffer for sending filename

int main() {
    //initialising server socket address structure with zero values
    bzero(&serv_addr,sizeof(serv_addr));
    
    //filling up the server socket address structure with appropriate values
    serv_addr.sin_family = AF_INET;// address family
    serv_addr.sin_port = htons(serv_port);// port number
    inet_aton(serv_ip, (&serv_addr.sin_addr));// IP address
    
    printf("\nTCP ECHO CLIENT\n");
    
    //creating socket
    if((skfd = socket(AF_INET, SOCK_STREAM, 0))<0) {
        printf("\nCLIENT ERROR: Cannot create socket.\n");
        exit(1);
    }
    
    //request server for a connection
    if((connect(skfd, (struct sockaddr*)&serv_addr, sizeof(serv_addr)))<0) {
        printf("\nCLIENT ERROR: Cannot connect to the server.\n");
        close(skfd);
        exit(1);
    }
    printf("\nCLIENT: Connected to the server.\n");

    // === CHANGE STARTS HERE ===
    printf("\nEnter filename to send: ");
    fgets(sbuff, sizeof(sbuff), stdin);
    sbuff[strcspn(sbuff, "\n")] = '\0'; // remove newline
    // === CHANGE ENDS HERE ===
    
    //send the filename to the server
    if((w=write(skfd,sbuff,strlen(sbuff)))<0) {
        printf("\nCLIENT ERROR: Cannot send message to the echo server\n");
        close(skfd);
        exit(1);
    }
    printf("\nCLIENT: Filename sent to server.\n");
    
    //read back the server response
    if((r=read(skfd,rbuff,128))<0)
        printf("\nCLIENT SERVER: Cannot receive message from the server\n");
    else {
        rbuff[r]='\0';
        
        //print the received message on console
        printf("\nCLIENT: Message from the server: %s\n",rbuff);
    }
    
    close(skfd);
    exit(1);
}//main ends

