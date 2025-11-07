/* UDP Echo Client Program */

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>

struct sockaddr_in serv_addr;


int skfd, r, w;
socklen_t serv_addr_len;

unsigned short serv_port = 25020;           /* Port number used by the server */
char serv_ip[] = "127.0.0.1";           /* Server's IP address */

char rbuff[128];                            /* Buffer for receiving messages */
char sbuff[128] = "===DEBAJYOTI KARMAKAR===";// buffer for sending messages

int main() {
    /* Initializing server socket address structure with zero values */
    bzero(&serv_addr, sizeof(serv_addr));

    /* Filling up the server socket address structure with appropriate values */
    serv_addr.sin_family = AF_INET;                     /* Address family */
    serv_addr.sin_port = htons(serv_port);              /* Port number */
    inet_aton(serv_ip, &serv_addr.sin_addr);            /* IP address */
    serv_addr_len = sizeof(serv_addr);

    printf("\nUDP ECHO CLIENT.\n");

    /* Creating socket */
    if ((skfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        perror("CLIENT ERROR: Cannot create socket");
        exit(1);
    }

    /* Send a message to the echo server */
    w = sendto(skfd, sbuff, strlen(sbuff), 0,
               (struct sockaddr *)&serv_addr, serv_addr_len);
    if (w < 0) {
        perror("CLIENT ERROR: Cannot send message to echo server");
        close(skfd);
        exit(1);
    }
    printf("CLIENT: Message sent to echo server.\n");

    /* Read back the echoed message from server */
    r = recvfrom(skfd, rbuff, sizeof(rbuff) - 1, 0,
                 (struct sockaddr *)&serv_addr, &serv_addr_len);
    if (r < 0) {
        perror("CLIENT ERROR: Cannot receive message from server");
    } else {
        rbuff[r] = '\0';  /* Null-terminate the received data */
        printf("CLIENT: Message from echo server: %s\n", rbuff);
    }

    close(skfd);
    return 0;
}

