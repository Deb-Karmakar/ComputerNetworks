// server.c
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>

#define BUFSZ 1024

struct sockaddr_in serv_addr, cli_addr;
int listenfd, connfd;
socklen_t cli_addr_len;
unsigned short serv_port = 25020;
char serv_ip[] = "127.0.0.1";

ssize_t readline_sock(int fd, char *out, size_t maxlen) {
    size_t n = 0;
    while (n + 1 < maxlen) {
        char c;
        ssize_t rc = read(fd, &c, 1);
        if (rc == 1) {
            out[n++] = c;
            if (c == '\n') break;
        } else {
            return rc; // 0 for EOF, -1 for error
        }
    }
    out[n] = '\0';
    return n;
}

ssize_t writen(int fd, const void *vptr, size_t n) {
    size_t nleft = n;
    const char *ptr = vptr;
    while (nleft > 0) {
        ssize_t nw = write(fd, ptr, nleft);
        if (nw <= 0) return nw;
        nleft -= nw;
        ptr += nw;
    }
    return n;
}

int main() {
    bzero(&serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(serv_port);
    inet_aton(serv_ip, &serv_addr.sin_addr);

    printf("TCP FILE SERVER\n");

    listenfd = socket(AF_INET, SOCK_STREAM, 0);
    if (listenfd < 0) {
        printf("SERVER ERROR: cannot create socket\n");
        return 1;
    }

    {
        int opt = 1;
        setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
    }

    if (bind(listenfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        printf("SERVER ERROR: bind failed\n");
        close(listenfd);
        return 1;
    }

    if (listen(listenfd, 5) < 0) {
        printf("SERVER ERROR: listen failed\n");
        close(listenfd);
        return 1;
    }

    cli_addr_len = sizeof(cli_addr);
    for (;;) {
        printf("\nSERVER: waiting for a client...\n");
        connfd = accept(listenfd, (struct sockaddr *)&cli_addr, &cli_addr_len);
        if (connfd < 0) {
            printf("SERVER ERROR: accept failed\n");
            break;
        }
        printf("SERVER: connection from %s\n", inet_ntoa(cli_addr.sin_addr));

        char line[BUFSZ];

        /* 1) Read prefix (line terminated by '\n') */
        if (readline_sock(connfd, line, sizeof(line)) <= 0) {
            close(connfd);
            continue;
        }
        // remove trailing newline
        line[strcspn(line, "\n")] = '\0';
        printf("SERVER: prefix request: '%s'\n", line);

        /* 2) List matching files */
        char cmd[BUFSZ];
        if (strlen(line) == 0) {
            snprintf(cmd, sizeof(cmd), "ls -1");
        } else {
            // grep -E '^prefix' ; escape ^ for grep pattern; prefix is used raw (ok for local testing)
            snprintf(cmd, sizeof(cmd), "ls -1 | grep -E '^%s'", line);
        }

        FILE *p = popen(cmd, "r");
        if (!p) {
            writen(connfd, "NOTFOUND\n", 9);
            close(connfd);
            continue;
        }

        int found = 0;
        writen(connfd, "LIST\n", 5);
        while (fgets(line, sizeof(line), p)) {
            // ensure newline present (fgets keeps newline), send as-is
            found = 1;
            writen(connfd, line, strlen(line));
        }
        pclose(p);

        if (!found) {
            writen(connfd, "NOTFOUND\n", 9);
            close(connfd);
            continue;
        }
        writen(connfd, "ENDLIST\n", 8);

        /* 3) Read chosen filename (line) */
        if (readline_sock(connfd, line, sizeof(line)) <= 0) {
            close(connfd);
            continue;
        }
        line[strcspn(line, "\n")] = '\0';
        printf("SERVER: client chose '%s'\n", line);

        FILE *f = fopen(line, "r");
        if (!f) {
            writen(connfd, "NOTFOUND\n", 9);
            close(connfd);
            continue;
        }

        /* 4) Send menu (each line ends with '\n') */
        writen(connfd, "MENU\n", 5);
        writen(connfd, "1) Print\n", 9);
        writen(connfd, "2) Save\n", 8);
        writen(connfd, "3) Print and Save\n", 18);
        writen(connfd, "ENDMENU\n", 8);

        /* 5) Read client's menu choice (line) */
        char choice[32];
        if (readline_sock(connfd, choice, sizeof(choice)) <= 0) {
            fclose(f);
            close(connfd);
            continue;
        }
        choice[strcspn(choice, "\n")] = '\0';
        printf("SERVER: choice = '%s'\n", choice);

        /* 6) Send filename header */
        char header[BUFSZ];
        snprintf(header, sizeof(header), "FILENAME:%s\n", line);
        writen(connfd, header, strlen(header));

        /* 7) Send file content line-by-line (text mode) */
        char filebuf[BUFSZ];
        while (fgets(filebuf, sizeof(filebuf), f)) {
            writen(connfd, filebuf, strlen(filebuf));
        }
        writen(connfd, "__END_OF_FILE__\n", 16);

        fclose(f);
        close(connfd);
        printf("SERVER: done sending '%s'\n", line);
    }

    close(listenfd);
    return 0;
}
