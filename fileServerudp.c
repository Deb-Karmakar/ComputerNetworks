#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#define MAXLINE 1024

struct sockaddr_in serv_addr, cli_addr;
unsigned short serv_port = 25020;
char buffer[MAXLINE];
char serv_ip[] = "127.0.0.1"; // server IP address

int file_exists(const char *filename);

int main()
{
    int sockfd;
    socklen_t cli_len;
    char filename[256];
    FILE *fp;

    // Create UDP socket
    if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
    {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }

    bzero(&serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(serv_port);
    inet_aton(serv_ip, &serv_addr.sin_addr);

    if (bind(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
    {
        perror("Bind failed");
        close(sockfd);
        exit(EXIT_FAILURE);
    }

    printf("UDP FILE SERVER STARTED ");

    cli_len = sizeof(cli_addr);

    while (1)
    {
        printf("\nWaiting for client filename request...\n");
        bzero(filename, sizeof(filename));

        // Receive filename from client
        int r = recvfrom(sockfd, filename, sizeof(filename) - 1, 0,
                         (struct sockaddr *)&cli_addr, &cli_len);
        if (r <= 0)
        {
            printf("Failed to receive filename from client.\n");
            continue;
        }
        filename[r] = '\0';

        printf("Requested file: %s\n", filename);

        if (!file_exists(filename))
        {
            char *msg = "ERROR: File not found on server.\n";
            sendto(sockfd, msg, strlen(msg), 0, (struct sockaddr *)&cli_addr, cli_len);
            printf("File not found: %s\n", filename);
            continue;
        }
        else
        {
            char *msg = "OK";
            sendto(sockfd, msg, strlen(msg), 0, (struct sockaddr *)&cli_addr, cli_len);
            printf("Sent OK message. Sending file data...\n");

            // Open and send file content without waiting for READY
            fp = fopen(filename, "r");
            if (fp == NULL)
            {
                perror("Failed to open file");
                continue;
            }

            while (fgets(buffer, MAXLINE, fp) != NULL)
            {
                sendto(sockfd, buffer, strlen(buffer), 0, (struct sockaddr *)&cli_addr, cli_len);
                usleep(1000); // Small delay to avoid overwhelming client
            }

            fclose(fp);
            printf("File '%s' sent to client.\n", filename);
        }
    }

    close(sockfd);
    return 0;
}

int file_exists(const char *filename)
{
    char cmd[512];
    char result[256];
    FILE *fp;

    sprintf(cmd, "ls | grep '^%s$'", filename);
    fp = popen(cmd, "r");
    if (fp == NULL)
    {
        perror("popen failed");
        return 0;
    }

    if (fgets(result, sizeof(result), fp) != NULL)
    {
        pclose(fp);
        return 1;
    }

    pclose(fp);
    return 0;
}

