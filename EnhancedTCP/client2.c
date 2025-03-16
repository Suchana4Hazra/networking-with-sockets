#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <pthread.h>

#define PORT 8080
#define BUFFER_SIZE 1024
volatile int running = 1; // Flag to indicate if the chat is running

void* send_message(void* arg) {
    int sockfd = *(int*)arg;
    int request;
    while (running) {
        printf("\nEnter Request:\n");
        printf("1. Query for the time of the server.\n");
        printf("2. Query for the time and date of the server.\n");
        printf("3. Query for the server name.\n");
        printf("4. Exit\n");
        printf("Choice: ");
        
        if (scanf("%d", &request) != 1) { // Handle invalid input
            printf("Invalid input. Please enter a number.\n");
            while (getchar() != '\n'); // Clear input buffer
            continue;
        }

        request = htonl(request); // Convert to network byte order
        send(sockfd, &request, sizeof(request), 0);

        if (ntohl(request) == 4) { // Exit if user chose 4
            running = 0;
            break;
        }
    }
    return NULL;
}

void* receive_message(void* arg) {
    int sockfd = *(int*)arg;
    char buffer[BUFFER_SIZE];
    
    while (running) {
        memset(buffer, 0, BUFFER_SIZE);
        int bytes_received = recv(sockfd, buffer, BUFFER_SIZE, 0);
        if (bytes_received > 0) {
            printf("\nServer response: %s", buffer);
            if (strcmp(buffer, "exit\n") == 0) {
                running = 0;
                break;
            }
        }
    }
    return NULL;
}

int main() {
    int sockfd;
    struct sockaddr_in server_addr;
    
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd == -1) {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }
    
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);
    server_addr.sin_addr.s_addr = inet_addr("127.0.0.1");

    if (connect(sockfd, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        perror("Connection failed");
        close(sockfd);
        exit(EXIT_FAILURE);
    }

    printf("Connected to server.\n");

    pthread_t send_thread, receive_thread;
    pthread_create(&send_thread, NULL, send_message, &sockfd);
    pthread_create(&receive_thread, NULL, receive_message, &sockfd);

    pthread_join(send_thread, NULL);
    running = 0;
    pthread_join(receive_thread, NULL);

    close(sockfd);
    printf("Client disconnected.\n");
    return 0;
}