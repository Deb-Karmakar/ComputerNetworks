/* UDP Iterative Chat Server Program */

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
unsigned short serv_port = 25050; /* Port number to be used by the server */
char serv_ip[] = "127.0.0.1"; /* Server's IP address */
char buff[128];          /* Buffer for receiving messages */
char server_msg[128];    /* Buffer for server messages */

int main() {
    /* Initialize server socket address structure with zero values */
    bzero(&serv_addr, sizeof(serv_addr));
    
    /* Fill up the server socket address structure with appropriate values */
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(serv_port);
    inet_aton(serv_ip, &serv_addr.sin_addr);

    printf("\nUDP Chat Server.\n");

    /* Create socket */
    if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        perror("SERVER ERROR: Cannot create socket");
        exit(1);
    }

    /* Bind socket */
    if (bind(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        perror("SERVER ERROR: Cannot bind");
        close(sockfd);
        exit(1);
    }

    cli_addr_len = sizeof(cli_addr);

    printf("SERVER: Listening for messages... Press Ctrl+C to stop.\n");

    for (;;) {
        /* Clear buffers */
        bzero(buff, sizeof(buff));
        bzero(server_msg, sizeof(server_msg));

        /* Receive message from client */
        r = recvfrom(sockfd, buff, sizeof(buff) - 1, 0,
                     (struct sockaddr *)&cli_addr, &cli_addr_len);
        if (r < 0) {
            perror("SERVER ERROR: Cannot receive message");
            continue;
        }

        buff[r] = '\0';

        printf("\nClient %s:%d: %s\n", inet_ntoa(cli_addr.sin_addr), ntohs(cli_addr.sin_port), buff);

        /* Check if client wants to stop */
        if (strcmp(buff, "stop") == 0) {
            printf("SERVER: Client %s:%d ended the chat session.\n", inet_ntoa(cli_addr.sin_addr), ntohs(cli_addr.sin_port));
            continue;
        }

        /* Get server's response */
        printf("Server: ");
        fgets(server_msg, sizeof(server_msg), stdin);

        /* Check if server wants to stop */
        if (strcmp(server_msg, "stop\n") == 0) {
            strcpy(server_msg, "stop");
            sendto(sockfd, server_msg, strlen(server_msg), 0,
                   (struct sockaddr *)&cli_addr, cli_addr_len);
            printf("SERVER: Chat session ended by server.\n");
            continue;
        }

        /* Send server message to client */
        w = sendto(sockfd, server_msg, strlen(server_msg), 0,
                   (struct sockaddr *)&cli_addr, cli_addr_len);
        if (w < 0) {
            perror("SERVER ERROR: Cannot send message to client");
        } else {
            printf("SERVER: Message sent to client %s:%d\n", inet_ntoa(cli_addr.sin_addr), ntohs(cli_addr.sin_port));
        }
    }

    close(sockfd);
    return 0;
}

