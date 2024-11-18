//client


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <time.h>

#define PORT 3000
#define TIMEOUT 2  
#define ACK "ACK"
#define DATA "Hiii....massage successfully reach."

void error(const char *msg) {
    perror(msg);
    exit(EXIT_FAILURE);
}

int main() {
    int sockfd;
    struct sockaddr_in serveraddr, recvaddr;
    socklen_t addrlen = sizeof(recvaddr);
    char buffer[1024];
    int n;
    fd_set readfds;
    struct timeval tv;
    int retry_limit = 3;
    int retry_count = 0;
    int ack_received = 0;

    if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        error("socket");
    }

    memset(&serveraddr, 0, sizeof(serveraddr));
    serveraddr.sin_family = AF_INET;
    serveraddr.sin_addr.s_addr = inet_addr("127.0.0.1");
    serveraddr.sin_port = htons(PORT);

    srand(time(NULL));

    while (retry_count < retry_limit && !ack_received) {
        printf("Sending: %s\n", DATA);
        if (sendto(sockfd, DATA, sizeof(DATA), 0, (struct sockaddr *)&serveraddr, sizeof(serveraddr)) < 0) {
            error("sendto");
        }

        FD_ZERO(&readfds);
        FD_SET(sockfd, &readfds);
        tv.tv_sec = TIMEOUT;
        tv.tv_usec = 0;

        n = select(sockfd + 1, &readfds, NULL, NULL, &tv);

        if (n == 0) {
            printf("Timeout. Retransmitting...\n");
            retry_count++;
        } else if (FD_ISSET(sockfd, &readfds)) {
            if (recvfrom(sockfd, buffer, sizeof(buffer), 0, (struct sockaddr *)&recvaddr, &addrlen) < 0) {
                error("recvfrom");
            }
            printf("Received ACK. Message was received correctly.\n");
            ack_received = 1;
        }
    }

    if (!ack_received) {
        printf("Failed to receive ACK after %d retries. Exiting...\n", retry_limit);
    }

    close(sockfd);
    return 0;
}


//server


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>

#define PORT 3000
#define ACK "ACK"

void error(const char *msg) {
    perror(msg);
    exit(EXIT_FAILURE);
}

int main() {
    int sockfd;
    struct sockaddr_in serveraddr, clientaddr;
    socklen_t addrlen = sizeof(clientaddr);
    char buffer[1024];

    // Create a UDP socket
    if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        error("socket");
    }

    // Configure the server address
    memset(&serveraddr, 0, sizeof(serveraddr));
    serveraddr.sin_family = AF_INET;
    serveraddr.sin_addr.s_addr = INADDR_ANY;
    serveraddr.sin_port = htons(PORT);

    // Bind the socket to the address
    if (bind(sockfd, (struct sockaddr *)&serveraddr, sizeof(serveraddr)) < 0) {
        error("bind");
    }

    while (1) {
        // Receive data bit
        if (recvfrom(sockfd, buffer, sizeof(buffer), 0, (struct sockaddr *)&clientaddr, &addrlen) < 0) {
            error("recvfrom");
        }

        printf("Received: %s\n", buffer);

        // Simulate noise by randomly deciding whether to send an ACK or not
        if (rand() % 3) {
            // Send ACK
            if (sendto(sockfd, ACK, sizeof(ACK), 0, (struct sockaddr *)&clientaddr, sizeof(clientaddr)) < 0) {
                error("sendto");
            }
            printf("Sent ACK.\n");
        } else {
            // Simulate no ACK (for noise)
            printf("Simulated noise: No ACK sent.\n");
        }
    }

    close(sockfd);
    return 0;
}
