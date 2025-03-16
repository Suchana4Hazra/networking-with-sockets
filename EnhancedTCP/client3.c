#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include "socketutil.h"

#define PORT 2000
#define BUFFER_SIZE 1024

void sendArrayToServer(int socketFD);

int main() {
    int socketFD = createTCPIpv4Socket();
    struct sockaddr_in *serverAddress = createIPv4Address("127.0.0.1", PORT);

    if (connect(socketFD, (struct sockaddr*)serverAddress, sizeof(struct sockaddr_in)) < 0) {
        perror("Connection failed");
        exit(1);
    }

    printf("Connected to the server.\n");
    sendArrayToServer(socketFD);

    close(socketFD);
    return 0;
}

void sendArrayToServer(int socketFD) {
    uint16_t num_elements;
    uint8_t buffer[2 + (BUFFER_SIZE * 4)];

    printf("Enter the number of elements in the array: ");
    scanf("%hu", &num_elements);

    uint16_t num_elements_network = htons(num_elements);
    memcpy(buffer, &num_elements_network, 2);

    uint32_t elements[num_elements];

    printf("Enter %d elements:\n", num_elements);
    for (int i = 0; i < num_elements; i++) {
        scanf("%u", &elements[i]);
        uint32_t temp = htonl(elements[i]);
        memcpy(buffer + 2 + (i * 4), &temp, 4);
    }

    send(socketFD, buffer, 2 + (num_elements * 4), 0);

    recv(socketFD, buffer, 2 + (num_elements * 4), 0);

    printf("Sorted array received from server:\n");
    for (int i = 0; i < num_elements; i++) {
        uint32_t temp;
        memcpy(&temp, buffer + 2 + (i * 4), 4);
        temp = ntohl(temp);
        printf("%u ", temp);
    }
    printf("\n");
}