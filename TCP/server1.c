#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define PORT 4455

void main() {
    int sockfd, newSocket;
    struct sockaddr_in serverAddr, newAddr;
    socklen_t addr_size;
    char buffer[1024];

    // Create server socket
    sockfd = socket(PF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        perror("[+]Server socket creation failed!");
        exit(1);
    }
    printf("[+]Server Socket Created Successfully\n");

    memset(&serverAddr, '\0', sizeof(serverAddr));

    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(PORT);
    serverAddr.sin_addr.s_addr = inet_addr("127.0.0.1");

    // Bind the server socket
    if (bind(sockfd, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) < 0) {
        perror("[+]Binding failed!");
        exit(1);
    }

    printf("[+]Bind to the port number: %d\n", PORT);
    listen(sockfd, 5);  // Listen for incoming connections
    printf("[+]Listening...\n");

    addr_size = sizeof(newAddr);
    newSocket = accept(sockfd, (struct sockaddr*)&newAddr, &addr_size);
    if (newSocket < 0) {
        perror("[+]Connection failed!");
        exit(1);
    }
    printf("[+]Client connected!\n");

    // Chat loop: Server and client send messages back and forth
    while (1) {
        // Receive message from client
        memset(buffer, '\0', sizeof(buffer));  // Clear buffer
        int bytesReceived = recv(newSocket, buffer, sizeof(buffer)-1, 0);
        if (bytesReceived <= 0) {
            printf("[+]Client disconnected or error occurred\n");
            break;
        }

        printf("Client: %s\n", buffer);

        // If client sends "bye", close the connection
        if (strncmp(buffer, "bye", 3) == 0) {
            printf("[+]Client ended the chat.\n");
            break;
        }

        // Server sends a response
        printf("Enter message for client: ");
        fgets(buffer, sizeof(buffer), stdin);
        send(newSocket, buffer, strlen(buffer), 0);

        // If server sends "bye", close the connection
        if (strncmp(buffer, "bye", 3) == 0) {
            printf("[+]Server ended the chat.\n");
            break;
        }
    }

    // Close the socket
    close(newSocket);
    close(sockfd);
    printf("[+]Server connection closed.\n");
}