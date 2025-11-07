#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#define MAXLINE 1024

struct sockaddr_in serv_addr;
int skfd;
unsigned short serv_port = 25020;
char serv_ip[] = "127.0.0.1"; // server IP address

int main()
{
    char buff[MAXLINE];
    char filename[256];
    FILE *fp = NULL;
    socklen_t serv_len = sizeof(serv_addr);

    printf("\nUDP FILE CLIENT\n");

    // Create UDP socket
    if ((skfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
    {
        perror("CLIENT ERROR: Cannot create socket");
        exit(EXIT_FAILURE);
    }

    bzero(&serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(serv_port);
    inet_aton(serv_ip, &serv_addr.sin_addr);

    // Get filename from user
    printf("Enter filename to request from server: ");
    fgets(filename, sizeof(filename), stdin);
    filename[strcspn(filename, "\n")] = '\0';

    // Send filename request to server
    if (sendto(skfd, filename, strlen(filename), 0,
               (struct sockaddr *)&serv_addr, serv_len) < 0)
    {
        perror("CLIENT ERROR: Failed to send filename");
        close(skfd);
        exit(EXIT_FAILURE);
    }

    // Receive server response ("OK" or "ERROR")
    int r = recvfrom(skfd, buff, MAXLINE - 1, 0,
                     (struct sockaddr *)&serv_addr, &serv_len);
    if (r <= 0)
    {
        printf("No response from server or connection closed.\n");
        close(skfd);
        exit(EXIT_FAILURE);
    }
    buff[r] = '\0';

    if (strncmp(buff, "ERROR", 5) == 0)
    {
        printf("SERVER: %s\n", buff);
        close(skfd);
        exit(EXIT_FAILURE);
    }

    printf("SERVER: %s\n", buff);

    int choice;
    printf("Choose an option:\n");
    printf("1. Print file contents to screen\n");
    printf("2. Save file locally\n");
    printf("Enter choice (1 or 2): ");
    scanf("%d", &choice);
    getchar(); // Consume newline

    if (choice == 2)
    {
        fp = fopen(filename, "w");
        if (fp == NULL)
        {
            perror("CLIENT ERROR: Cannot open file to save");
            close(skfd);
            exit(EXIT_FAILURE);
        }
        printf("Saving file as '%s'\n", filename);
    }
    else
    {
        printf("Printing file contents:\n\n");
    }

    printf("Receiving file data...\n");

    // Receive file data packets until timeout or no more data
    struct timeval tv;
    tv.tv_sec = 3;  // timeout of 3 seconds
    tv.tv_usec = 0;
    setsockopt(skfd, SOL_SOCKET, SO_RCVTIMEO, (const char*)&tv, sizeof(tv));

    while (1)
    {
        r = recvfrom(skfd, buff, MAXLINE - 1, 0,
                     (struct sockaddr *)&serv_addr, &serv_len);
        if (r <= 0)
        {
            // Timeout or no data - assume transmission finished
            break;
        }
        buff[r] = '\0';

        if (choice == 2)
            fputs(buff, fp);
        else
            printf("%s", buff);
    }

    if (fp)
    {
        fflush(fp);
        fclose(fp);
    }

    printf("\nFile transfer completed.\n");

    close(skfd);
    printf("Connection closed.\n");

    return 0;
}

