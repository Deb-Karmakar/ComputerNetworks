#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>

struct sockaddr_in serv_addr, cli_addr;

int listenfd, connfd, r, w, cli_addr_len;

unsigned short serv_port = 25020; // port number to be used by the server
char serv_ip[] = "127.0.0.1"; // server IP address

char buff[120]; // buffer for sending and receiving messages

int main() {
    // Initializing server socket address structure with zero values
    bzero(&serv_addr, sizeof(serv_addr));

    // Filling up the server socket address structure with appropriate values
    serv_addr.sin_family = AF_INET; // address family
    serv_addr.sin_port = htons(serv_port); // port number
    inet_aton(serv_ip, &serv_addr.sin_addr); // IP address

    printf("\nTCP ECHO SERVER.\n");

    // Creating socket
    if ((listenfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        printf("\nSERVER ERROR: cannot create socket.\n");
        exit(1);
    }

    // Binding server socket address structure
    if (bind(listenfd, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0) {
        printf("\nSERVER ERROR: Cannot bind.\n");
        close(listenfd);
        exit(1);
    }

    // Listen to client connection requests
    if (listen(listenfd, 5) < 0) {
        printf("\nSERVER ERROR: Cannot listen.\n");
        close(listenfd);
        exit(1);
    }

    cli_addr_len = sizeof(cli_addr);
    for (;;) {
        printf("\nSERVER: Listening for clients... Press Ctrl + C to stop the server.\n");
        
        // Accept client connections
        if ((connfd = accept(listenfd, (struct sockaddr*)&cli_addr, &cli_addr_len)) < 0) {
            printf("\nSERVER ERROR: Cannot accept client connections\n");
            close(listenfd);
            exit(1);
        }
        
        printf("\nSERVER: Connection from client %s accepted.\n", inet_ntoa(cli_addr.sin_addr));
        
        // Waiting for messages from client
        if ((r = read(connfd, buff, sizeof(buff) - 1)) < 0) {
            printf("\nSERVER ERROR: Cannot receive message from client.\n");
        } else {
            buff[r] = '\0';
            
            // Echo back the message received from client 
            if ((w = write(connfd, buff, r)) < 0) {
                printf("\nSERVER ERROR: Cannot send message to the client.\n");
            } else {
                printf("\nSERVER: Echoed back %s to %s.\n", buff, inet_ntoa(cli_addr.sin_addr));
            }
        }
        
        close(connfd); // Close the connection after handling the client
    } // for ends
} // main ends

