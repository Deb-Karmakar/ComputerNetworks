#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>

struct sockaddr_in serv_addr, cli_addr;

int listenfd, connfd;
ssize_t r, w;
socklen_t cli_addr_len;

unsigned short serv_port = 25020; // port number to be used by the server
char serv_ip[] = "127.0.0.1"; // server IP address

/* remove trailing newline/carriage return */
void trim_newline(char *s) {
    size_t i = strlen(s);
    while (i > 0 && (s[i-1] == '\n' || s[i-1] == '\r')) {
        s[i-1] = '\0';
        --i;
    }
}

int main() {
    bzero(&serv_addr, sizeof(serv_addr));

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(serv_port);
    inet_aton(serv_ip, &serv_addr.sin_addr);

    printf("\nTCP ECHO SERVER.\n");

    if ((listenfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("SERVER ERROR: cannot create socket");
        exit(1);
    }

    if (bind(listenfd, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0) {
        perror("SERVER ERROR: Cannot bind");
        close(listenfd);
        exit(1);
    }

    if (listen(listenfd, 5) < 0) {
        perror("SERVER ERROR: Cannot listen");
        close(listenfd);
        exit(1);
    }

    for (;;) {
        cli_addr_len = sizeof(cli_addr);
        printf("\nSERVER: Listening for clients... Press Ctrl + C to stop the server.\n");
        fflush(stdout);

        if ((connfd = accept(listenfd, (struct sockaddr*)&cli_addr, &cli_addr_len)) < 0) {
            perror("SERVER ERROR: Cannot accept client connections");
            continue;
        }
        printf("\nSERVER: Connection from client %s accepted.\n", inet_ntoa(cli_addr.sin_addr));

        for (;;) {
            char buff[256];   /* incoming */
            char sbuff[256];  /* outgoing */

            r = read(connfd, buff, sizeof(buff) - 1);
            if (r < 0) {
                perror("SERVER ERROR: Cannot receive message from client");
                break;
            } else if (r == 0) {
                printf("Client disconnected.\n");
                break;
            }

            buff[r] = '\0';
            trim_newline(buff);
            printf("Client Message: %s\n", buff);

            if (strcmp(buff, "STOP") == 0) {
                printf("Stopping the connection as client requested STOP.\n");
                break;
            }

            printf("Enter Server's message: ");
            fflush(stdout);
            if (fgets(sbuff, sizeof(sbuff), stdin) == NULL) {
                /* EOF on stdin - close connection */
                printf("No input (stdin closed). Closing connection.\n");
                break;
            }
            trim_newline(sbuff);

            /* Send only the meaningful bytes */
            w = write(connfd, sbuff, strlen(sbuff));
            if (w < 0) {
                perror("SERVER ERROR: Cannot send message to the client");
                break;
            } else {
                printf("SERVER: Echoed back \"%s\" to %s.\n", sbuff, inet_ntoa(cli_addr.sin_addr));
            }

            if (strcmp(sbuff, "STOP") == 0) {
                printf("Stopping the connection as server sent STOP.\n");
                break;
            }
        }

        close(connfd);
    }

    close(listenfd);
    return 0;
}

