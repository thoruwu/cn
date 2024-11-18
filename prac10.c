#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

#define BUF_SIZE 1024

void error(const char *msg) {
    perror(msg);
    exit(1);
}

int main() {
    int sockfd, newsockfd, portno, n;
    char buffer[BUF_SIZE];
    struct sockaddr_in serv_addr, cli_addr;
    socklen_t clilen;
    int totalclients = 5, idx;
    FILE *file;
    char filename[20];
    
    portno = 8080;

    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        error("ERROR opening socket");
    }

    bzero((char *) &serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(portno);

    if (bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) {
        error("ERROR on binding");
    }

    listen(sockfd, totalclients);
    clilen = sizeof(cli_addr);

    int fileno = 0;
    for (idx = 0; idx < totalclients; idx++) {
        newsockfd = accept(sockfd, (struct sockaddr *) &cli_addr, &clilen);
        if (newsockfd < 0) {
            error("ERROR on accept");
        }

        printf("Connected with client %d\n", idx + 1);

        snprintf(filename, sizeof(filename), "mit%d.txt", fileno++);
        file = fopen(filename, "w");
        if (file == NULL) {
            error("ERROR opening file");
        }

        bzero(buffer, BUF_SIZE);
        while ((n = recv(newsockfd, buffer, BUF_SIZE, 0)) > 0) {
            fwrite(buffer, sizeof(char), n, file);
            bzero(buffer, BUF_SIZE);
        }

        if (n < 0) {
            error("ERROR reading from socket");
        }

        printf("Receiving file from client %d\n", idx + 1);
        printf("Received successfully! New filename is: %s\n", filename);
        fclose(file);

        close(newsockfd);
    }

    close(sockfd);
    return 0;
}


//client
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define BUF_SIZE 1024

void error(const char *msg) {
    perror(msg);
    exit(1);
}

int main() {
    int sockfd, n;
    struct sockaddr_in serv_addr;
    char buffer[BUF_SIZE];
    char filename[256];
    FILE *file;

    // Server address setup
    const char *host = "127.0.0.1";
    int port = 8080;

    // Create socket
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        error("ERROR opening socket");
    }

    bzero((char *) &serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(port);


    if (inet_pton(AF_INET, host, &serv_addr.sin_addr) <= 0) {
        error("ERROR invalid host address");
    }

    // Connect to the server
    if (connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        error("ERROR connecting to server");
    }

    while (1) {
        printf("Input filename you want to send: ");
        scanf("%s", filename);

        file = fopen(filename, "r");
        if (file == NULL) {
            printf("You entered an invalid filename! Please enter a valid name\n");
            continue; 
        }

        printf("Sending file %s to server...\n", filename);
        while ((n = fread(buffer, 1, BUF_SIZE, file)) > 0) {
            if (send(sockfd, buffer, n, 0) < 0) {
                error("ERROR sending data to server");
            }
        }

        printf("File sent successfully!\n");

        fclose(file);
        break;  
    }

    close(sockfd);
    return 0;
}


