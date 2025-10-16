// client.c
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>

#define BUFSZ 1024

struct sockaddr_in serv_addr;
int skfd;
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
            return rc; // 0 EOF, -1 error
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

int file_exists(const char *path) {
    return access(path, F_OK) == 0;
}

int main() {
    bzero(&serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(serv_port);
    inet_aton(serv_ip, &serv_addr.sin_addr);

    printf("TCP FILE CLIENT\n");

    skfd = socket(AF_INET, SOCK_STREAM, 0);
    if (skfd < 0) {
        printf("CLIENT ERROR: cannot create socket\n");
        return 1;
    }

    if (connect(skfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        printf("CLIENT ERROR: cannot connect\n");
        close(skfd);
        return 1;
    }
    printf("CLIENT: Connected to server\n");

    char line[BUFSZ];

    /* 1) send prefix (line + newline) */
    printf("\nEnter filename or prefix to search: ");
    fflush(stdout);
    if (!fgets(line, sizeof(line), stdin)) {
        close(skfd);
        return 0;
    }
    // ensure newline at end (fgets keeps '\n'), send full line
    writen(skfd, line, strlen(line));

    /* 2) expect LIST or NOTFOUND */
    if (readline_sock(skfd, line, sizeof(line)) <= 0) { close(skfd); return 0; }
    if (strncmp(line, "NOTFOUND", 8) == 0) {
        printf("CLIENT: No matching files\n");
        close(skfd);
        return 0;
    }
    if (strncmp(line, "LIST", 4) == 0) {
        printf("\nCLIENT: Files matching prefix:\n");
        while (1) {
            if (readline_sock(skfd, line, sizeof(line)) <= 0) { close(skfd); return 0; }
            if (strncmp(line, "ENDLIST", 7) == 0) break;
            printf(" - %s", line); // line includes newline
        }
    } else {
        printf("CLIENT: Unexpected response: %s\n", line);
        close(skfd);
        return 0;
    }

    /* 3) choose exact filename and send (with newline) */
    printf("\nEnter exact filename to request: ");
    fflush(stdout);
    if (!fgets(line, sizeof(line), stdin)) { close(skfd); return 0; }
    // Remove trailing newline and re-add to ensure exactly one newline
    line[strcspn(line, "\n")] = '\0';
    strcat(line, "\n");
    writen(skfd, line, strlen(line)); // send chosen filename

    /* 4) read MENU or NOTFOUND */
    if (readline_sock(skfd, line, sizeof(line)) <= 0) { close(skfd); return 0; }
    if (strncmp(line, "NOTFOUND", 8) == 0) {
        printf("CLIENT: Server says file not found\n");
        close(skfd);
        return 0;
    }
    if (strncmp(line, "MENU", 4) == 0) {
        printf("\nCLIENT: Server Menu:\n");
        while (1) {
            if (readline_sock(skfd, line, sizeof(line)) <= 0) { close(skfd); return 0; }
            if (strncmp(line, "ENDMENU", 7) == 0) break;
            printf("%s", line); // already contains newline
        }
    } else {
        printf("CLIENT: Unexpected menu response: %s\n", line);
        close(skfd);
        return 0;
    }

    /* 5) get user's choice and send with exactly one newline */
    char choice[32];
    printf("\nEnter choice (1=Print, 2=Save, 3=Print and Save): ");
    fflush(stdout);
    if (!fgets(choice, sizeof(choice), stdin)) { close(skfd); return 0; }
    // Remove trailing newline and re-add to ensure exactly one newline
    choice[strcspn(choice, "\n")] = '\0';
    strcat(choice, "\n");
    printf("CLIENT: Sending choice '%c'...\n", choice[0]);
    writen(skfd, choice, strlen(choice));

    /* 6) read FILENAME header */
    printf("CLIENT: Waiting for filename header...\n");
    if (readline_sock(skfd, line, sizeof(line)) <= 0) { 
        printf("CLIENT ERROR: Failed to read filename header\n");
        close(skfd); 
        return 0; 
    }
    
    char filename[BUFSZ];
    if (strncmp(line, "FILENAME:", 9) == 0) {
        strncpy(filename, line + 9, sizeof(filename)-1);
        filename[strcspn(filename, "\n")] = '\0';
    } else {
        strcpy(filename, "received_file");
    }
    printf("CLIENT: Server will send file: '%s'\n", filename);

    /* 7) prepare save filename (avoid overwrite) */
    char savefile[BUFSZ];
    strncpy(savefile, filename, sizeof(savefile)-1);
    savefile[sizeof(savefile)-1] = '\0';
    if (choice[0] == '2' || choice[0] == '3') {
        int cnt = 1;
        while (file_exists(savefile)) {
            snprintf(savefile, sizeof(savefile), "%s_copy%d", filename, cnt++);
        }
    }

    FILE *out = NULL;
    if (choice[0] == '2' || choice[0] == '3') {
        out = fopen(savefile, "w");
        if (out) printf("CLIENT: Will save to '%s'\n", savefile);
        else printf("CLIENT: Failed to open '%s' for writing (will still print if chosen)\n", savefile);
    }

    /* 8) receive file contents until __END_OF_FILE__\n */
    printf("\nCLIENT: Receiving file contents...\n\n");
    while (1) {
        if (readline_sock(skfd, line, sizeof(line)) <= 0) break;
        if (strcmp(line, "__END_OF_FILE__\n") == 0) break;
        if (choice[0] == '1' || choice[0] == '3') {
            printf("%s", line);
        }
        if (out) fputs(line, out);
    }

    if (out) {
        fclose(out);
        printf("\nCLIENT: File saved to '%s'\n", savefile);
    }
    printf("CLIENT: Transfer complete.\n");
    close(skfd);
    return 0;
}
