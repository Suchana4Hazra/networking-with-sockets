#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <unistd.h>
#include <pthread.h>
#include <arpa/inet.h>
#include "socketutil.h"

#define PORT 2000
#define BUFFER_SIZE 1024
#define MAX_CLIENTS 10

struct AcceptedSocket {
    int socketFD;
    struct sockaddr_in address;
};

struct AcceptedSocket acceptedSockets[MAX_CLIENTS];
int acceptedSocketsCount = 0;

void* handleClient(void* clientSocket);
void bubbleSort(uint32_t* arr, uint16_t size);
struct AcceptedSocket* acceptIncomingConnection(int serverSocketFD);
void startAcceptingIncomingConnections(int serverSocketFD);

int main() {
    int serverSocketFD = createTCPIpv4Socket();
    struct sockaddr_in *serverAddress = createIPv4Address("127.0.0.1", PORT);

    if (bind(serverSocketFD, (struct sockaddr*)serverAddress, sizeof(struct sockaddr_in)) < 0) {
        perror("Bind failed");
        exit(EXIT_FAILURE);
    }

    if (listen(serverSocketFD, MAX_CLIENTS) < 0) {
        perror("Listen failed");
        exit(EXIT_FAILURE);
    }

    printf("Server is listening on port %d...\n", PORT);

    startAcceptingIncomingConnections(serverSocketFD);

    close(serverSocketFD);
    free(serverAddress); // Free dynamically allocated memory
    return 0;
}

void startAcceptingIncomingConnections(int serverSocketFD) {
    while (true) {
        struct AcceptedSocket* clientSocket = acceptIncomingConnection(serverSocketFD);
        if (!clientSocket) {
            continue;
        }

        acceptedSockets[acceptedSocketsCount++] = *clientSocket;

        pthread_t thread;
        pthread_create(&thread, NULL, handleClient, &clientSocket->socketFD);
        pthread_detach(thread);

        free(clientSocket); // Free dynamically allocated memory after copying data
    }
}

struct AcceptedSocket* acceptIncomingConnection(int serverSocketFD) {
    struct sockaddr_in clientAddress;
    socklen_t addrSize = sizeof(clientAddress);
    int clientSocketFD = accept(serverSocketFD, (struct sockaddr*)&clientAddress, &addrSize);
    
    if (clientSocketFD < 0) {
        perror("Accept failed");
        return NULL;
    }

    struct AcceptedSocket* acceptedSocket = malloc(sizeof(struct AcceptedSocket));
    acceptedSocket->socketFD = clientSocketFD;
    acceptedSocket->address = clientAddress;

    printf("New client connected!\n");
    return acceptedSocket;
}

void* handleClient(void* clientSocket) {
    int socketFD = *(int*)clientSocket;
    uint8_t buffer[2 + (BUFFER_SIZE * 4)]; // 2 Bytes count + 4 Bytes per element

    while (true) {
        int bytesReceived = recv(socketFD, buffer, sizeof(buffer), 0);
        if (bytesReceived <= 0) {
            printf("Client disconnected\n");
            break;
        }

        // Extract number of elements (first 2 bytes)
        uint16_t num_elements;
        memcpy(&num_elements, buffer, 2);
        num_elements = ntohs(num_elements);
        printf("Received %d elements\n", num_elements);

        // Extract elements (4-byte integers)
        uint32_t elements[num_elements];
        for (int i = 0; i < num_elements; i++) {
            memcpy(&elements[i], buffer + 2 + (i * 4), 4);
            elements[i] = ntohl(elements[i]);
        }

        printf("Received array of size %d from client\n", num_elements);
        bubbleSort(elements, num_elements);

        // Copy sorted elements back to the buffer
        for (int i = 0; i < num_elements; i++) {
            uint32_t sortedValue = htonl(elements[i]);
            memcpy(buffer + 2 + (i * 4), &sortedValue, 4);
        }

        send(socketFD, buffer, 2 + (num_elements * 4), 0);
    }

    close(socketFD);
    return NULL;
}

// Bubble Sort
void bubbleSort(uint32_t* arr, uint16_t size) {
    for (int i = 0; i < size - 1; i++) {
        for (int j = 0; j < size - i - 1; j++) {
            if (arr[j] > arr[j + 1]) {
                uint32_t temp = arr[j];
                arr[j] = arr[j + 1];
                arr[j + 1] = temp;
            }
        }
    }
}