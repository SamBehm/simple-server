#include <stdio.h>
#include <stdlib.h>
#include <netdb.h>
#include <signal.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/time.h>
#include <sys/ioctl.h>
#include <stdarg.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <errno.h>


#define PORT 18000
#define SA struct sockaddr
#define MAX_STR 1000

int main(int argc, char** argv) {
    
    int sockfd;
    int n;
    int wbytes; // bytes to write
    struct sockaddr_in servaddr;

    char send[MAX_STR];
    char receive[MAX_STR];
    
    // Usage checking
    if (argc != 2) {
        fprintf(stderr, "Usage: ./client <ip_address>\n");
        fflush(stderr);
        exit(1);
    }

    // Attempt to create TCP socket. Die on error.
    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        fprintf(stderr, "Error creating the socket. Terminating.\n");
        fflush(stderr);
        exit(1);
    }

    // Zero servaddr struct and assign member values for TCP.
    bzero(&servaddr, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(PORT); // big-endian stuff

    // Attempt to convert IP address to binary representation
    if (inet_pton(AF_INET, argv[1], &servaddr.sin_addr) <= 0) {
        fprintf(stderr, "Could not convert IP address to binary. "
                "Terminating.\n");
        fflush(stderr);
        exit(1);
    }

    // Attempt to connect to given address. Die on error.
    if (connect(sockfd, (SA*) &servaddr, sizeof(servaddr)) < 0) {
        fprintf(stderr, "Connection failed. Terminating.\n");
        fflush(stderr);
        exit(2);
    }
    
    // GET request for root page.
    sprintf(send, "GET / HTTP/1.1\r\n\r\n");
    wbytes = strlen(send);

    if (write(sockfd, send, wbytes) != wbytes) {
        fprintf(stderr, "Failed to send full request. Terminating\n");
        fflush(stderr);
        exit(3);
    }
    
    // Null terminating strings are important
    memset(receive, 0, MAX_STR);
    while ((n = read(sockfd, receive, MAX_STR-1)) > 0) {
        printf("%s", receive);
        if (receive[n - 1] == '\n') {
            break;
        }
        memset(receive, 0, MAX_STR); // again, make sure no information is
                                     // repeated and null terminate your
                                     // strings
    }
    
    if (n < 0) {
        fprintf(stderr, "Error while reading / Nothing was read. "
                "Terminating\n");
        fprintf(stderr, "n = %i\n", n);
        fflush(stderr);
        exit(3);
    }

    // DONE!!
    close(sockfd);
    exit(0);
}
