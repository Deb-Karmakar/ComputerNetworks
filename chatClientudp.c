/* UDP Iterative Chat Client Program */

#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

struct sockaddr_in serv_addr;
int skfd, r, w;
socklen_t serv_addr_len;

unsigned short serv_port = 25050;        /* Server port number */
char serv_ip[] = "127.0.0.1";        /* Server IP address */
char buff[128];                          /* Buffer for receiving messages */
char client_msg[128];                    /* Buffer for client messages */

int main() {
    /* Initialize server address structure */
    bzero(&serv_addr, sizeof(serv_addr));

    /* Fill server address structure */
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(serv_port);
    inet_aton(serv_ip, &serv_addr.sin_addr);
    serv_addr_len = sizeof(serv_addr);

    printf("\nUDP ITERATIVE CHAT CLIENT.\n");

    /* Create socket */
    if ((skfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        perror("CLIENT ERROR: Cannot create socket");
        exit(1);
    }

    printf("CLIENT: Chat session started with server %s:%d. Type 'stop' to end session.\n", serv_ip, serv_port);

    while (1) {
        /* Clear buffers */
        bzero(client_msg, sizeof(client_msg));
        bzero(buff, sizeof(buff));

        /* Get client message */
        printf("\nClient: ");
        fgets(client_msg, sizeof(client_msg), stdin);

        /* Remove newline character */
        client_msg[strcspn(client_msg, "\n")] = '\0';

        /* Send message to server */
        w = sendto(skfd, client_msg, strlen(client_msg), 0,
                   (struct sockaddr *)&serv_addr, serv_addr_len);
        if (w < 0) {
            perror("CLIENT ERROR: Cannot send message to server");
            break;
        }

        /* Check if client wants to stop */
        if (strcmp(client_msg, "stop") == 0) {
            printf("CLIENT: Chat session ended by client.\n");
            break;
        }

        /* Receive server's response */
        r = recvfrom(skfd, buff, sizeof(buff) - 1, 0,
                     (struct sockaddr *)&serv_addr, &serv_addr_len);
        if (r < 0) {
            perror("CLIENT ERROR: Cannot receive message from server");
            break;
        }

        buff[r] = '\0';

        /* Check if server wants to stop */
        if (strcmp(buff, "stop") == 0) {
            printf("CLIENT: Chat session ended by server.\n");
            break;
        }

        printf("Server: %s", buff);
    }

    /* Close socket */
    close(skfd);
    printf("\nCLIENT: Connection closed.\n");
    return 0;
}

