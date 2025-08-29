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
    bzero(&serv_addr,sizeof(serv_addr));
    
    serv_addr.sin_family = AF_INET;// address family
    serv_addr.sin_port = htons(serv_port);// port number
    inet_aton(serv_ip, (&serv_addr.sin_addr));// IP address
    
    printf("\nTCP ECHO CLIENT\n");
    
    if((skfd = socket(AF_INET, SOCK_STREAM, 0))<0) {
        printf("\nCLIENT ERROR: Cannot create socket.\n");
        exit(1);
    }
    
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
    
    if((w=write(skfd,sbuff,strlen(sbuff)))<0) {
        printf("\nCLIENT ERROR: Cannot send message to the echo server\n");
        close(skfd);
        exit(1);
    }
    printf("\nCLIENT: Filename sent to server.\n");
    
    // === CHANGE STARTS HERE ===
    FILE *out = fopen("received_output.txt", "a"); // append mode
    if (!out) {
        printf("\nCLIENT ERROR: Cannot open output file.\n");
        close(skfd);
        exit(1);
    }

    int first_read = 1;
    while((r=read(skfd,rbuff,sizeof(rbuff)-1)) > 0) {
        rbuff[r]='\0';

        if(first_read) {
            if(strcmp(rbuff,"NOT FOUND")==0) {
                printf("\nCLIENT: Server says file not found.\n");
                fclose(out);
                close(skfd);
                return 0;
            }
            first_read = 0;
        }

        fputs(rbuff, out);   // append data to new file
        printf("%s", rbuff); // also show on console
    }
    fclose(out);
    printf("\nCLIENT: File received and saved to received_output.txt\n");
    // === CHANGE ENDS HERE ===
    
    close(skfd);
    exit(1);
}//main ends

