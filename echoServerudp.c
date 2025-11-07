/* UDP ECHO server program */

#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

struct sockaddr_in serv_addr, cli_addr;

int sockfd, r, w;
socklen_t cli_addr_len;

unsigned short serv_port = 25020;
char serv_ip[] = "127.0.0.1";
char buff[128];

int main() {
    /* Initializing server socket address structure with zero values */
    bzero(&serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;  /* Address family */
    serv_addr.sin_port = htons(serv_port);
    inet_aton(serv_ip, &serv_addr.sin_addr);

    printf("\nUDP ECHO SERVER.\n");

    /* Creating socket */
    if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        perror("SERVER ERROR: Cannot create socket");
        exit(1);
    }

    /* Binding server socket address structure */
    if (bind(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        perror("SERVER ERROR: Cannot bind");
        close(sockfd);
        exit(1);
    }

    cli_addr_len = sizeof(cli_addr);
    
    for (;;) {
        printf("\nSERVER LISTENING FOR CLIENT MESSAGES (Press Ctrl+C to stop):\n");

        /* Receiving message from client */
        r = recvfrom(sockfd, buff, sizeof(buff) - 1, 0,
                     (struct sockaddr *)&cli_addr, &cli_addr_len);
        if (r < 0) {
            perror("SERVER ERROR: Cannot receive message from client");
            continue;
        }

        buff[r] = '\0'; // Null terminate received data
        printf("SERVER: Received '%s' from %s:%d\n",
               buff,
               inet_ntoa(cli_addr.sin_addr),
               ntohs(cli_addr.sin_port));

        /* Echo back the message to client */
        w = sendto(sockfd, buff, r, 0,
                   (struct sockaddr *)&cli_addr, cli_addr_len);
        if (w < 0) {
            perror("SERVER ERROR: Cannot send message to client");
        } else {
            printf("SERVER: Echoed back '%s' to %s:%d\n",
                   buff,
                   inet_ntoa(cli_addr.sin_addr),
                   ntohs(cli_addr.sin_port));
        }
    }

    close(sockfd);
    return 0;
}

