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
    bzero(&serv_addr, sizeof(serv_addr));

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(serv_port);
    inet_aton(serv_ip, &serv_addr.sin_addr);

    printf("\nTCP FILE SERVER.\n");

    if ((listenfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        printf("\nSERVER ERROR: cannot create socket.\n");
        exit(1);
    }

    if (bind(listenfd, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0) {
        printf("\nSERVER ERROR: Cannot bind.\n");
        close(listenfd);
        exit(1);
    }

    if (listen(listenfd, 5) < 0) {
        printf("\nSERVER ERROR: Cannot listen.\n");
        close(listenfd);
        exit(1);
    }

    cli_addr_len = sizeof(cli_addr);
    for (;;) {
        printf("\nSERVER: Listening for clients... Press Ctrl + C to stop the server.\n");

        if ((connfd = accept(listenfd, (struct sockaddr*)&cli_addr, &cli_addr_len)) < 0) {
            printf("\nSERVER ERROR: Cannot accept client connections\n");
            close(listenfd);
            exit(1);
        }

        printf("\nSERVER: Connection from client %s accepted.\n", inet_ntoa(cli_addr.sin_addr));

        if ((r = read(connfd, buff, sizeof(buff) - 1)) < 0) {
            printf("\nSERVER ERROR: Cannot receive message from client.\n");
        } else {
            buff[r] = '\0';

            // === CHANGE STARTS HERE ===
            FILE *fp = fopen(buff, "r");
            if (fp == NULL) {
                strcpy(buff, "NOT FOUND");
                write(connfd, buff, strlen(buff));
                printf("\nSERVER: File not found.\n");
            } else {
                printf("\nSERVER: Sending file contents of %s to client...\n", buff);

                while (fgets(buff, sizeof(buff), fp) != NULL) {
                    w = write(connfd, buff, strlen(buff));
                    if (w < 0) {
                        printf("\nSERVER ERROR: Cannot send file data.\n");
                        break;
                    }
                }

                fclose(fp);
                printf("\nSERVER: File transfer complete.\n");
            }
            // === CHANGE ENDS HERE ===
        }

        close(connfd);
    }
}

