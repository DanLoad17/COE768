/* A simple echo server using TCP */
#include <stdio.h>
#include <sys/types.h>
#include <sys/unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/signal.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <strings.h>
#include <unistd.h> //////////////
#include <string.h> //////////////


#define SERVER_TCP_PORT 3000
#define BUFLEN 256

int echod(int);
void reaper(int);

int main(int argc, char **argv) {
    int sd, new_sd, client_len, port;
    struct sockaddr_in server, client;

    switch (argc) {
        case 1:
            port = SERVER_TCP_PORT;
            break;
        case 2:
            port = atoi(argv[1]);
            break;
        default:
            fprintf(stderr, "Usage: %s [port]\n", argv[0]);
            exit(1);
    }

    if ((sd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        fprintf(stderr, "Can't create a socket\n");
        exit(1);
    }

    bzero((char *)&server, sizeof(struct sockaddr_in));
    server.sin_family = AF_INET;
    server.sin_port = htons(port);
    server.sin_addr.s_addr = htonl(INADDR_ANY);

    if (bind(sd, (struct sockaddr *)&server, sizeof(server)) == -1) {
        fprintf(stderr, "Can't bind name to socket\n");
        exit(1);
    }

    listen(sd, 5);
    (void)signal(SIGCHLD, reaper);

    while (1) {
        client_len = sizeof(client);
        new_sd = accept(sd, (struct sockaddr *)&client, &client_len);
        if (new_sd < 0) {
            fprintf(stderr, "Can't accept client\n");
            exit(1);
        }

        switch (fork()) {
            case 0:
                (void)close(sd);
                exit(echod(new_sd));
            default:
                (void)close(new_sd);
                break;
            case -1:
                fprintf(stderr, "fork: error\n");
        }
    }
}

int echod(int sd) {
    char *bp, buf[BUFLEN];
    int n, bytes_to_read;

    while (n = read(sd, buf, BUFLEN)) { 
        char filename[BUFLEN]; //After accepting a connection from the client, the server receives the filename requested by the client and attempts to open the file.
				//If the filename is not found or the file cannot be opened, an error message is sent back to the client.
        if (read(sd, filename, BUFLEN) < 0) {
            perror("Error reading filename");
            write(sd, "Failed to read filename", strlen("Failed to read filename"));
            close(sd);
            exit(1);
        }

        FILE *file = fopen(filename, "rb");
        if (!file) {
            perror("Error opening file");
            write(sd, "EFile not found or could not be opened", strlen("EFile not found or could not be opened"));
            close(sd);
            exit(1); //Added error handling for situations where file reads or connections fail. In such cases, appropriate error messages are sent to the client, and the server exits.
        }

        char buffer[BUFLEN]; //After successfully opening the requested file, the server reads the file in chunks and sends the data to the client. This is done in a loop to transmit the entire file.
        while (1) {
            int bytes_read = fread(buffer, 1, BUFLEN, file);
            if (bytes_read <= 0) {
                break;
            }
            write(sd, buffer, bytes_read);
        }
        fclose(file);
    } //Added code to properly close the file and the connection when the file transmission is complete or when an error message is sent.

    close(sd);
    exit(0);
}

void reaper(int sig) {
    int status;
    while (wait3(&status, WNOHANG, (struct rusage *)0) >= 0);
}