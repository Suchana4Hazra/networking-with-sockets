#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>

#define PORT 4455
#define MAX_BUFFER_SIZE 1024

int main() {
    int sockfd, newSocket;
    struct sockaddr_in serverAddr, newAddr;
    socklen_t addr_size = sizeof(newAddr);
    char buffer[MAX_BUFFER_SIZE];
    char strinp[MAX_BUFFER_SIZE];

    // Create socket
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        perror("[-]Socket creation failed");
        exit(1);
    }
    printf("[+]Server Socket created successfully\n");

    // Setup server address
    memset(&serverAddr, 0, sizeof(serverAddr));
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(PORT);
    serverAddr.sin_addr.s_addr = INADDR_ANY; //all available interfaces

    // Bind socket to address
    if (bind(sockfd, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) < 0) {
        perror("[-]Bind failed");
        exit(1);
    }
    printf("[+]Binding to Port Number %d\n", PORT);

    // Listen for connections
    if (listen(sockfd, 5) < 0) {
        perror("[-]Listen failed");
        exit(1);
    }
    printf("[+]Listening...\n");

    // Accept connection
    newSocket = accept(sockfd, (struct sockaddr*)&newAddr, &addr_size);
    if (newSocket < 0) {
        perror("[-]Accept failed");
        exit(1);
    }
    printf("[+]Client connected\n");

    // Receive data from client (if needed)
    recv(newSocket, buffer, MAX_BUFFER_SIZE, 0);
    printf("Client data: %s\n", buffer);

    if(strcmp(buffer, "What's the time?")){
	time_t t;
    struct tm *tm_info;

    // Get the current time
    time(&t);

    // Convert to local time
    tm_info = localtime(&t);

    // Print the date in YYYY-MM-DD format
    sprintf(buffer,"%04d-%02d-%02d",
           tm_info->tm_year + 1900, // Year since 1900
           tm_info->tm_mon + 1,     // Month (0-based, so add 1)
           tm_info->tm_mday);  
    }
    // Send data to client
    send(newSocket, buffer, strlen(buffer), 0);
    printf("[+]Data sent to client\n");

    // Close sockets
    close(newSocket);
    close(sockfd);

    return 0;
}

server_time.c
Displaying server_time.c.