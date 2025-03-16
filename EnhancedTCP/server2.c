#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <pthread.h>

#define PORT 8080
#define BUFFER_SIZE 1024

void* handle_client(void* arg) {
    int client_fd = *(int*)arg;
    int request;
    
    while (1) {
        int bytes_received = recv(client_fd, &request, sizeof(request), 0);
        if (bytes_received <= 0) {
            printf("Client disconnected.\n");
            break;
        }

        request = ntohl(request); // Convert network byte order to host byte order
        char response[BUFFER_SIZE];

        if (request == 1) {
            snprintf(response, BUFFER_SIZE, "Server time: 19:42:16\n");
        } else if (request == 2) {
            snprintf(response, BUFFER_SIZE, "Server Date & time: 25-02-06 19:42:49\n");
        } else if (request == 3) {
            snprintf(response, BUFFER_SIZE, "Server Name: suchana-VirtualBox4\n");
        } else if (request == 4) {
            snprintf(response, BUFFER_SIZE, "exit\n");
            send(client_fd, response, strlen(response), 0);
            printf("Client requested exit. Closing connection.\n");
            break;
        } else {
            snprintf(response, BUFFER_SIZE, "Unsupported Request\n");
        }

        send(client_fd, response, strlen(response), 0);
        printf("Sent to client: %s", response); // Log message after sending response
    }
    
    close(client_fd);
    return NULL;
}

int main() {
    int server_fd, client_fd;
    struct sockaddr_in server_addr, client_addr;
    socklen_t addr_len = sizeof(client_addr);
    
    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd == -1) {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }
    
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(PORT);
    
    if (bind(server_fd, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        perror("Bind failed");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    if (listen(server_fd, 5) < 0) {
        perror("Listen failed");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    printf("Waiting for client to connect...\n");
    client_fd = accept(server_fd, (struct sockaddr*)&client_addr, &addr_len);
    if (client_fd < 0) {
        perror("Accept failed");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    printf("Client connected!\n");

    pthread_t client_thread;
    pthread_create(&client_thread, NULL, handle_client, &client_fd);
    pthread_join(client_thread, NULL);
    
    close(server_fd);
    printf("Server shutting down.\n");
    return 0;
}