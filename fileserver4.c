
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>

struct sockaddr_in serv_addr, cli_addr;

int listenfd, connfd, r, w;
int cli_addr_len;

unsigned short serv_port = 25020; // port number
char serv_ip[] = "127.0.0.1";     // server IP

char buff[120];
char filebuf[1024];

int main() {
    // Initialize server address
    bzero(&serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(serv_port);
    inet_aton(serv_ip, &serv_addr.sin_addr);

    printf("\nTCP SIMPLE FILE SERVER ");

    // Create socket
    if ((listenfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        printf("\nSERVER ERROR: cannot create socket.\n");
        exit(1);
    }

    // Bind
    if (bind(listenfd, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0) {
        printf("\nSERVER ERROR: Cannot bind.\n");
        close(listenfd);
        exit(1);
    }

    // Listen
    if (listen(listenfd, 5) < 0) {
        printf("\nSERVER ERROR: Cannot listen.\n");
        close(listenfd);
        exit(1);
    }

    cli_addr_len = sizeof(cli_addr);

    for (;;) {
        printf("\nSERVER: Listening for clients... Press Ctrl + C to stop the server.\n");

        // Accept
        if ((connfd = accept(listenfd, (struct sockaddr*)&cli_addr, (socklen_t*)&cli_addr_len)) < 0) {
            printf("\nSERVER ERROR: Cannot accept client connections\n");
            close(listenfd);
            exit(1);
        }

        printf("\nSERVER: Connection from client %s accepted.\n", inet_ntoa(cli_addr.sin_addr));

        // Receive filename
        r = read(connfd, buff, sizeof(buff) - 1);
        if (r <= 0) {
            printf("SERVER: client closed connection or read error.\n");
            close(connfd);
            continue;
        }
        buff[r] = '\0';
        if (r > 0 && (buff[r-1] == '\n' || buff[r-1] == '\r')) buff[r-1] = '\0';
        if (r > 1 && (buff[r-2] == '\r')) buff[r-2] = '\0';

        char fname[120];
        snprintf(fname, sizeof(fname), "%s", buff);
        printf("SERVER: client requested file: '%s'\n", fname);

        // Open file
        FILE *f = fopen(fname, "r");
        if (f == NULL) {
            snprintf(buff, sizeof(buff), "NOTFOUND\n");
            w = write(connfd, buff, strlen(buff));
            if (w < 0)
                printf("\nSERVER ERROR: Cannot send NOTFOUND.\n");
            else
                printf("SERVER: Sent NOTFOUND to client.\n");
            close(connfd);
            continue;
        }

        // Send menu
        snprintf(buff, sizeof(buff), "MENU\n");
        write(connfd, buff, strlen(buff));
        snprintf(buff, sizeof(buff), "1) Print\n");
        write(connfd, buff, strlen(buff));
        snprintf(buff, sizeof(buff), "2) Save\n");
        write(connfd, buff, strlen(buff));
        snprintf(buff, sizeof(buff), "3) Print and Save\n");
        write(connfd, buff, strlen(buff));
        snprintf(buff, sizeof(buff), "ENDMENU\n");
        write(connfd, buff, strlen(buff));

        // Read client choice
        r = read(connfd, buff, sizeof(buff) - 1);
        if (r <= 0) {
            printf("SERVER: client closed connection before choice.\n");
            fclose(f);
            close(connfd);
            continue;
        }
        buff[r] = '\0';
        if (r > 0 && (buff[r-1] == '\n' || buff[r-1] == '\r')) buff[r-1] = '\0';
        if (r > 1 && (buff[r-2] == '\r')) buff[r-2] = '\0';

        printf("SERVER: client choice = '%s'\n", buff);

        int do_print = 0, do_save = 0;
        if (strcmp(buff, "1") == 0) do_print = 1;
        else if (strcmp(buff, "2") == 0) do_save = 1;
        else if (strcmp(buff, "3") == 0) { do_print = 1; do_save = 1; }

        FILE *savef = NULL;
        char savename[256];
        if (do_save) {
            snprintf(savename, sizeof(savename), "saved_%s", fname);
            savef = fopen(savename, "w");
            if (savef == NULL) {
                printf("SERVER: could not open save file '%s'.\n", savename);
                do_save = 0;
            } else {
                printf("SERVER: will save to '%s'\n", savename);
            }
        }

        // Send filename header
        snprintf(buff, sizeof(buff), "FILENAME:%s\n", fname);
        write(connfd, buff, strlen(buff));

        // Send file line by line
        while (fgets(filebuf, sizeof(filebuf), f)) {
            w = write(connfd, filebuf, strlen(filebuf));
            if (w < 0) {
                printf("SERVER ERROR: Cannot send file data.\n");
                break;
            }
            if (do_print) fputs(filebuf, stdout);
            if (do_save && savef) fputs(filebuf, savef);
        }

        snprintf(filebuf, sizeof(filebuf), "__END_OF_FILE__\n");
        write(connfd, filebuf, strlen(filebuf));

        if (savef) fclose(savef);
        fclose(f);

        printf("SERVER: done sending '%s'\n", fname);
        close(connfd);
    }

    close(listenfd);
    return 0;
}

