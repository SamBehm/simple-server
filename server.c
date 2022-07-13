#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <signal.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#define PORT 18000
#define SA struct sockaddr
#define MAX_LINE 4096

static volatile int keepRunning = 1;

// void intHandler()
// SIGINT handler. Used so that variables can be cleaned up on temrination.
//
// Parameters:
// int var - dummy variable
void intHandler(int var) {
    keepRunning = 0;
}

int main(int argc, char** argv) {

    int sock, connectionfd, n;
    socklen_t addrlen;
    struct sockaddr_in servaddr, clientaddr;
    uint8_t buffer[MAX_LINE];
    uint8_t readBuffer[MAX_LINE];

    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        fprintf(stderr, "Could not create socket. Termimnating\n");
        exit(1);
    }
    
    // Make sure that addresses can be reused.
    const int enable = 1;
    if (setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(int)) < 0) {
        fprintf(stderr, "setsockopt(SO_REUSEADDR) failed.\n");
    }

    bzero(&servaddr, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servaddr.sin_port = htons(PORT);
    
    if ((bind(sock, (SA *) &servaddr, sizeof(servaddr))) < 0) {
        fprintf(stderr, "Could not assign address to socket. Terminating\n");
        exit(1);
    }

    if ((listen(sock, 10)) < 0) {
        fprintf(stderr, "Could not listen on port %d. Terminating.\n", PORT);
        exit(1);
    }

    // Infinite connection loop. Only 1 connection is accepted at a time for now.
    // (Multi-threading will be implemented later)
    while (keepRunning) {
        addrlen = sizeof(struct sockaddr_in);
        printf("Listening for a connection...\n");
        fflush(stdout);
        
        connectionfd = accept(sock, (SA *) &clientaddr, &addrlen);
        
        memset(readBuffer, 0, MAX_LINE);
        
        struct sockaddr_in* v4Addr = (struct sockaddr_in*) &clientaddr;
        struct in_addr ipAddress = v4Addr->sin_addr;
        
        char str[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, &ipAddress, str, INET_ADDRSTRLEN);


        printf("New Connection:\nsin_family: %hi\nsin_port: %hi\nsin_addr: %s\n",
                clientaddr.sin_family,
                ntohs(clientaddr.sin_port),
                str);
        
        // Read request/message from client.
        memset(readBuffer, 0, MAX_LINE);
        while ((n = read(connectionfd, readBuffer, MAX_LINE-1)) > 0) {
            fprintf(stdout, "%s", readBuffer);
            
            if (readBuffer[n - 1] == '\n') {
                break;
            }

            memset(readBuffer, 0, MAX_LINE);
        }
        printf("\n");

        if (n < 0) {
            fprintf(stderr, "Could not read request.\n");
        } else {
            snprintf((char*) buffer, sizeof(buffer), "HTTP/1.0 200 OK\r\n\r\nHello!\r\n");
            write(connectionfd, (char*) buffer, strlen((char*) buffer)); 
            memset(buffer, 0, MAX_LINE);
            close(connectionfd);
        }
     
    }
    close(sock);
    close(connectionfd);
}
