
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
unsigned short serv_port = 25020; // port number used by the server
char serv_ip[] = "127.0.0.1";     // server IP Address

char rbuff[1024]; // buffer for receiving messages
char sbuff[128];  // buffer for sending filename or choice

int main() {
    // initializing server socket address structure
    bzero(&serv_addr, sizeof(serv_addr));

    // filling up the server socket address structure
    serv_addr.sin_family = AF_INET;              // address family
    serv_addr.sin_port = htons(serv_port);       // port number
    inet_aton(serv_ip, (&serv_addr.sin_addr));   // IP address

    printf("\nTCP FILE CLIENT\n");

    // creating socket
    if ((skfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        printf("\nCLIENT ERROR: Cannot create socket.\n");
        exit(1);
    }

    // connect to the server
    if ((connect(skfd, (struct sockaddr*)&serv_addr, sizeof(serv_addr))) < 0) {
        printf("\nCLIENT ERROR: Cannot connect to the server.\n");
        close(skfd);
        exit(1);
    }
    printf("\nCLIENT: Connected to the server.\n");

    // Ask user for filename
    printf("\nEnter filename to request: ");
    if (fgets(sbuff, sizeof(sbuff), stdin) == NULL) {
        close(skfd);
        exit(1);
    }
    sbuff[strcspn(sbuff, "\n")] = '\0'; // remove newline

    // send the filename
    if ((w = write(skfd, sbuff, strlen(sbuff))) < 0) {
        printf("\nCLIENT ERROR: Cannot send filename to the server.\n");
        close(skfd);
        exit(1);
    }
    printf("\nCLIENT: Filename '%s' sent to server.\n", sbuff);

    // Variables to control client-side saving/printing
    int client_do_save = 0;
    int client_do_print = 0;
    FILE *savef = NULL;
    char recv_fname[256];

    // Read server replies
    while ((r = read(skfd, rbuff, sizeof(rbuff) - 1)) > 0) {
        rbuff[r] = '\0';

        // If server says NOTFOUND
        if (strncmp(rbuff, "NOTFOUND", 8) == 0) {
            printf("\nCLIENT: Server says file not found.\n");
            break;
        }

        // If we received the MENU
        if (strstr(rbuff, "MENU") != NULL) {
            printf("\nCLIENT: Received menu from server:\n");
            printf("%s", rbuff);

            // Continue reading until we see "ENDMENU"
            while (strstr(rbuff, "ENDMENU") == NULL) {
                r = read(skfd, rbuff, sizeof(rbuff) - 1);
                if (r <= 0) break;
                rbuff[r] = '\0';
                printf("%s", rbuff);
            }

            // Ask user for choice
            printf("\nEnter your choice (1/2/3): ");
            if (fgets(sbuff, sizeof(sbuff), stdin) == NULL) {
                break;
            }
            sbuff[strcspn(sbuff, "\n")] = '\0';

            // Determine client-side behavior
            if (strcmp(sbuff, "1") == 0) { client_do_print = 1; client_do_save = 0; }
            else if (strcmp(sbuff, "2") == 0) { client_do_save = 1; client_do_print = 0; }
            else if (strcmp(sbuff, "3") == 0) { client_do_save = 1; client_do_print = 1; }

            // Send choice to server
            write(skfd, sbuff, strlen(sbuff));
            printf("CLIENT: Choice '%s' sent.\n", sbuff);
            continue;
        }

        // If file transfer header
        if (strncmp(rbuff, "FILENAME:", 9) == 0) {
            // Extract filename after "FILENAME:"
            char *p = rbuff + 9;
            // Trim newline or spaces
            char *nl = strpbrk(p, "\r\n");
            if (nl) *nl = '\0';
            snprintf(recv_fname, sizeof(recv_fname), "%s", p);
            printf("\nCLIENT: Receiving file from server...\n");
            printf("Header: FILENAME:%s\n\n", recv_fname);

            // If client wants to save, open a local file named received_<filename>
            if (client_do_save) {
                char save_name[300];
                snprintf(save_name, sizeof(save_name), "received_%s", recv_fname);
                savef = fopen(save_name, "w");
                if (savef == NULL) {
                    printf("CLIENT: Could not open local file '%s' for writing. Will not save.\n", save_name);
                    client_do_save = 0;
                } else {
                    printf("CLIENT: Saving to local file '%s'\n", save_name);
                }
            }

            // There may be file content in the same buffer after the header.
            // Check for __END_OF_FILE__ marker in current rbuff.
            char *marker = strstr(rbuff, "__END_OF_FILE__");
            if (marker != NULL) {
                // if marker present, print/save the part after header up to marker (if any)
                char *content_start = strstr(rbuff, "\n");
                if (content_start) {
                    content_start++; // skip first newline
                    // compute content length up to marker
                    size_t len = (size_t)(marker - content_start);
                    if (len > 0) {
                        // temporarily null-terminate for printing
                        char temp = content_start[len];
                        ((char *)content_start)[len] = '\0';
                        if (client_do_print) printf("%s", content_start);
                        if (client_do_save && savef) {
                            fwrite(content_start, 1, len, savef);
                        }
                        ((char *)content_start)[len] = temp;
                    }
                }
                printf("\nCLIENT: File transfer complete.\n");
                if (savef) fclose(savef);
                break;
            }

            // Otherwise, keep reading successive chunks until marker arrives
            while (1) {
                r = read(skfd, rbuff, sizeof(rbuff) - 1);
                if (r <= 0) {
                    printf("\nCLIENT: Connection closed unexpectedly.\n");
                    if (savef) fclose(savef);
                    break;
                }
                rbuff[r] = '\0';

                // Check if this chunk contains the end marker
                marker = strstr(rbuff, "__END_OF_FILE__");
                if (marker != NULL) {
                    // print/save data up to marker
                    size_t content_len = (size_t)(marker - rbuff);
                    if (content_len > 0) {
                        // print
                        if (client_do_print) {
                            // ensure we don't print extra bytes after marker
                            char tmp = rbuff[content_len];
                            ((char *)rbuff)[content_len] = '\0';
                            printf("%s", rbuff);
                            ((char *)rbuff)[content_len] = tmp;
                        }
                        if (client_do_save && savef) {
                            fwrite(rbuff, 1, content_len, savef);
                        }
                    }
                    printf("\nCLIENT: File transfer complete.\n");
                    if (savef) fclose(savef);
                    break;
                } else {
                    // full chunk: print and/or save whole chunk
                    if (client_do_print) printf("%s", rbuff);
                    if (client_do_save && savef) fwrite(rbuff, 1, strlen(rbuff), savef);
                }
            } // end inner read loop

            break; // done receiving file; exit outer loop
        } // end if FILENAME
    } // end while read

    close(skfd);
    printf("\nCLIENT: Connection closed.\n");
    return 0;
}

