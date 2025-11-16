#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>

struct sockaddr_in serv_addr;

int skfd;
ssize_t r, w;

unsigned short serv_port = 25020; // port number used by the server
char serv_ip[] = "127.0.0.1"; // server IP Address

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

    printf("\nTCP ECHO CLIENT\n");

    if ((skfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("CLIENT ERROR: Cannot create socket");
        exit(1);
    }

    if (connect(skfd, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0) {
        perror("CLIENT ERROR: Cannot connect to the server");
        close(skfd);
        exit(1);
    }
    printf("\nCLIENT: Connected to the server.\n");

    for (;;) {
        char sbuff[256];
        char rbuff[256];

        printf("Enter Client's message:\n");
        fflush(stdout);
        if (fgets(sbuff, sizeof(sbuff), stdin) == NULL) {
            printf("No input (stdin closed). Exiting.\n");
            break;
        }
        trim_newline(sbuff);

        printf("\nCLIENT: Message to the server: %s\n", sbuff);

        /* send only the meaningful bytes */
        w = write(skfd, sbuff, strlen(sbuff));
        if (w < 0) {
            perror("CLIENT ERROR: Cannot send message to the server");
            close(skfd);
            exit(1);
        }

        if (strcmp(sbuff, "STOP") == 0) {
            printf("Stopping the connection as client sent STOP.\n");
            break;
        }

        /* read server reply */
        r = read(skfd, rbuff, sizeof(rbuff) - 1);
        if (r < 0) {
            perror("CLIENT ERROR: Cannot receive message from the server");
            break;
        } else if (r == 0) {
            printf("Server closed connection.\n");
            break;
        }
        rbuff[r] = '\0';
        trim_newline(rbuff);

        printf("\nCLIENT: Message from the server: %s\n", rbuff);

        if (strcmp(rbuff, "STOP") == 0) {
            printf("Stopping the connection as server sent STOP.\n");
            break;
        }
    }

    close(skfd);
    return 0;
}

