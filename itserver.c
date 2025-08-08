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
char serv_ip[] = "192.168.21.25"; // server IP address

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
        perror("SERVER ERROR: cannot create socket");
        exit(1);
    }

    // Binding server socket address structure
    if (bind(listenfd, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0) {
        perror("SERVER ERROR: Cannot bind");
        close(listenfd);
        exit(1);
    }

    // Listen to client connection requests
    if (listen(listenfd, 5) < 0) {
        perror("SERVER ERROR: Cannot listen");
        close(listenfd);
        exit(1);
    }

    cli_addr_len = sizeof(cli_addr);
    for (;;) {
        printf("\nSERVER: Listening for clients... Press Ctrl + C to stop the server.\n");
        
        // Accept client connections
        if ((connfd = accept(listenfd, (struct sockaddr*)&cli_addr, &cli_addr_len)) < 0) {
            perror("SERVER ERROR: Cannot accept client connections");
            continue; // Continue to listen for other clients
        }
        printf("\nSERVER: Connection from client %s accepted.\n", inet_ntoa(cli_addr.sin_addr));
      
        for (;;) { // For Iterative Chat
            char buff[120]; // for reading client message
            char sbuff[120]; // for sending server message
            
            // Waiting for messages from client
            if ((r = read(connfd, buff, sizeof(buff) - 1)) < 0) {
                perror("SERVER ERROR: Cannot receive message from client");
                break; // Exit the loop on error
            } else if (r == 0) {
                printf("Client disconnected.\n");
                break; // Exit the loop if client disconnects
            }
            buff[r] = '\0'; // Null-terminate the received string
            printf("Client Message: %s\n", buff); // print client message
            
            // Stop the connection if the client sends "STOP"
            if (strcmp(buff, "STOP") == 0) {
                printf("Stopping the connection...\n");
                break; // Exit the loop
            }
           
            printf("Enter Server's message: "); // scans the server's message
            fgets(sbuff, sizeof(sbuff), stdin);
            
            // Send back the message to client received from server
            if ((w = write(connfd, sbuff, strlen(sbuff))) < 0) {
                perror("SERVER ERROR: Cannot send message to the client");
                break; // Exit the loop on error
            } else {
                printf("SERVER: Echoed back %s to %s.\n", sbuff, inet_ntoa(cli_addr.sin_addr));
            }
        }
        close(connfd); // Close the connection with the client
    }

    return 0;
}

