#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#define PORT 4455
#define MAX_BUFFER_SIZE 1024

int main() {
    int sockfd;
    struct sockaddr_in serverAddr;
    char buffer[MAX_BUFFER_SIZE] = "What's the time?";

    // Create socket
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        perror("[-]Socket creation failed");
        exit(1);
    }
    printf("[+]Client Socket created successfully\n");

    // Setup server address
    memset(&serverAddr, 0, sizeof(serverAddr));
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(PORT);
    serverAddr.sin_addr.s_addr = inet_addr("127.0.0.1"); // Connect to localhost

    // Connect to the server
    if (connect(sockfd, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) < 0) {
        perror("[-]Connection failed");
        exit(1);
    }
    printf("[+]Connected to Server\n");

    // Send data to server
    send(sockfd, buffer, strlen(buffer), 0);
    printf("[+]Data sent to server: %s\n", buffer);

    // Receive data from server
    memset(buffer, 0, sizeof(buffer));
    recv(sockfd, buffer, MAX_BUFFER_SIZE, 0);
    printf("[+]Server response: %s\n", buffer);

    // Close socket
    close(sockfd);

    return 0;
}
