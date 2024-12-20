/* A simple echo client using TCP */
#include <stdio.h>
#include <netdb.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <unistd.h>
#include <strings.h>
#include <string.h> ///////////////
#include <fcntl.h> ////////////////

#define SERVER_TCP_PORT 3000
#define BUFLEN 256

int main(int argc, char **argv) {
    int n, i, bytes_to_read;
    int sd, port;
    struct hostent *hp;
    struct sockaddr_in server;
    char *host, *bp, rbuf[BUFLEN], sbuf[BUFLEN];

    switch (argc) {
        case 2:
            host = argv[1];
            port = SERVER_TCP_PORT;
            break;
        case 3:
            host = argv[1];
            port = atoi(argv[2]);
            break;
        default:
            fprintf(stderr, "Usage: %s host [port]\n", argv[0]);
            exit(1);
    }

    if ((sd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        fprintf(stderr, "Can't create a socket\n");
        exit(1);
    }

    bzero((char *)&server, sizeof(struct sockaddr_in));
    server.sin_family = AF_INET;
    server.sin_port = htons(port);

    if (hp = gethostbyname(host))
        bcopy(hp->h_addr, (char *)&server.sin_addr, hp->h_length);
    else if (inet_aton(host, (struct in_addr *)&server.sin_addr)) {
        fprintf(stderr, "Can't get server's address\n");
        exit(1);
    }

    if (connect(sd, (struct sockaddr *)&server, sizeof(server)) == -1) {
        fprintf(stderr, "Can't connect\n");
        exit(1);
    }

    printf("Enter the filename to download: "); ////////////////prompt shows up
    scanf("%s", sbuf); //enter the filename that we want to download
    write(sd, sbuf, strlen(sbuf) + 1);

    while (n = read(sd, sbuf, BUFLEN)) { //Modified the part of the code responsible for reading data from the server.
        bp = rbuf;
        bytes_to_read = n;
        while ((i = read(sd, bp, bytes_to_read)) > 0) {
            if (bp[0] == 'E') { //Checked the first byte of received data to determine whether it's an error message or file data.
                printf("Error: %s\n", bp + 1);
            } else { //If it's not an error message, the data is assumed to be part of the file, and it is saved to a local file
                int fd = open("downloaded_file.txt", O_WRONLY | O_CREAT, S_IRUSR | S_IWUSR);
                write(fd, bp, i);
                close(fd);
            }
            break;
        } //Added code to properly exit the client when the server closes the connection.
    }

    close(sd); 
    exit(0);
}