// TCP ITERATIVE FILE SERVER
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
char serv_ip[] = "127.0.0.1";     
char buffer[MAXLINE];

int file_exists(const char *filename);

int main()
{
    int listenfd, connfd;
    socklen_t cli_len;
    char filename[256];
    FILE *fp;

    
    if ((listenfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        printf("Socket creation failed");
        exit(EXIT_FAILURE);
    }

    
    bzero(&serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(serv_port);
    inet_aton(serv_ip, &serv_addr.sin_addr);

    
    if (bind(listenfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
    {
        printf("Bind failed");
        close(listenfd);
        exit(EXIT_FAILURE);
    }

    // Listen
    if (listen(listenfd, 5) < 0)
    {
        printf("Listen failed");
        close(listenfd);
        exit(EXIT_FAILURE);
    }

    printf("TCP FILE SERVER STARTED on IP %s, PORT %d...\n", serv_ip, serv_port);

    while (1)
    {
        cli_len = sizeof(cli_addr);
        printf("\nWaiting for client connection...\n");

        if ((connfd = accept(listenfd, (struct sockaddr *)&cli_addr, &cli_len)) < 0)
        {
            printf("Accept failed");
            continue;
        }

        printf("Connected to client: %s\n", inet_ntoa(cli_addr.sin_addr));

        
        bzero(filename, sizeof(filename));
        int r = read(connfd, filename, sizeof(filename) - 1);
        if (r <= 0)
        {
            printf("Failed to receive filename from client.\n");
            close(connfd);
            continue;
        }
        filename[r] = '\0';

        printf("Requested file: %s\n", filename);

       
        if (!file_exists(filename))
        {
            char *msg = "ERROR: File not found on server.\n";
            write(connfd, msg, strlen(msg));
            printf("File not found: %s\n", filename);
        }
        else
        {
           
            char *msg = "OK\n";
            write(connfd, msg, strlen(msg));
            printf("Sent OK message. Waiting for client READY...\n");

       
            char ack[32];
            bzero(ack, sizeof(ack));
            int ack_r = read(connfd, ack, sizeof(ack) - 1);
            if (ack_r <= 0 || strncmp(ack, "READY", 5) != 0)
            {
                printf("Client did not respond with READY. Aborting.\n");
                close(connfd);
                continue;
            }

            
            fp = fopen(filename, "r");
            if (fp == NULL)
            {
                printf("Failed to open file");
                close(connfd);
                continue;
            }

            while (fgets(buffer, MAXLINE, fp) != NULL)
            {
                write(connfd, buffer, strlen(buffer));
            }

            fclose(fp);
            printf("File '%s' sent to client.\n", filename);
        }

        close(connfd);
        printf("Connection closed with client.\n");
    }

    close(listenfd);
    return 0;
}

// Function to check if file exists using shell command
int file_exists(const char *filename)
{
    char cmd[512];
    char result[256];
    FILE *fp;

    sprintf(cmd, "ls | grep '^%s$'", filename);
    fp = popen(cmd, "r");
    if (fp == NULL)
    {
        printf("popen failed");
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

