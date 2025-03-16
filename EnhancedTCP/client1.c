#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>
#include <unistd.h>
#include "socketutil.h"

#define BUFFER_SIZE 1024

volatile int running = 1; // Flag to indicate if the chat is running

void* send_message(void* arg) {
    int sockfd = *(int*)arg;
    char buffer[BUFFER_SIZE];
    while (running) {
	printf("\n Enter message to server: \n");
        fgets(buffer, BUFFER_SIZE, stdin);
        if (strncmp(buffer, "exit", 4) == 0) {
            running = 0;
            break;
        }
        send(sockfd, buffer, strlen(buffer), 0);
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
            printf("Friend: %s", buffer);
        }
    }
    return NULL;
}

void run_server() {
    int server_fd = createTCPIpv4Socket();
    struct sockaddr_in *server_addr = createIPv4Address("", 8080);
    bind(server_fd, (struct sockaddr*)server_addr, sizeof(struct sockaddr_in));
    listen(server_fd, 1);
    
    printf("Waiting for client to connect...\n");
    int client_fd = accept(server_fd, NULL, NULL);
    printf("Client connected!\n");
    
    pthread_t send_thread, receive_thread;
    pthread_create(&send_thread, NULL, send_message, &client_fd);
    pthread_create(&receive_thread, NULL, receive_message, &client_fd);
    
    pthread_join(send_thread, NULL);
    running = 0;
    pthread_join(receive_thread, NULL);
    
    close(client_fd);
    close(server_fd);
    free(server_addr);
    
    printf("Server shutting down.\n");
}

void run_client() {
    int sockfd = createTCPIpv4Socket();
    struct sockaddr_in *server_addr = createIPv4Address("127.0.0.1", 8080);
    
    if (connect(sockfd, (struct sockaddr*)server_addr, sizeof(struct sockaddr_in)) < 0) {
        perror("Connection failed");
        exit(EXIT_FAILURE);
    }
    
    pthread_t send_thread, receive_thread;
    pthread_create(&send_thread, NULL, send_message, &sockfd);
    pthread_create(&receive_thread, NULL, receive_message, &sockfd);
    
    pthread_join(send_thread, NULL);
    running = 0;
    pthread_join(receive_thread, NULL);
    
    close(sockfd);
    free(server_addr);
    
    printf("Client disconnected.\n");
}

int main() {


        run_client();
}