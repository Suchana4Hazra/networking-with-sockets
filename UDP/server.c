#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <stdint.h>

#define MAX_SIZE 1000

void send_error_packet(int server_fd, struct sockaddr_in *client_addr, socklen_t addr_len, uint16_t sequence_number, uint8_t error_code) {
    uint8_t error_packet[4];
    error_packet[0] = error_code; // Message Type for error packet
    uint16_t network_sequence_number = htons(sequence_number);
    memcpy(error_packet + 1, &network_sequence_number, 2);
    error_packet[3] = error_code;
    sendto(server_fd, error_packet, sizeof(error_packet), 0, (struct sockaddr *)client_addr, addr_len);
}

int main(int argc, char *argv[]) {
    int server_fd;
    struct sockaddr_in address, client_addr;
    socklen_t addr_len = sizeof(client_addr);
    uint8_t buffer[8 + MAX_SIZE]; // Packet buffer: 1 byte MT + 2 bytes SN + 1 byte TTL + 4 bytes PL + P bytes PB
    char payload[MAX_SIZE];

    if (argc != 2) {
        fprintf(stderr, "Usage: %s <port>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    // Create UDP socket
    if ((server_fd = socket(AF_INET, SOCK_DGRAM, 0)) == 0) {
        perror("Socket failed");
        exit(EXIT_FAILURE);
    }

    // Bind to PORT
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = inet_addr("127.0.0.1");
    address.sin_port = htons(atoi(argv[1]));
    
    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        perror("Bind failed");
        exit(EXIT_FAILURE);
    }

    printf("Server listening on port %s...\n", argv[1]);

    while (1) {
        int valread = recvfrom(server_fd, buffer, sizeof(buffer), 0, (struct sockaddr *)&client_addr, &addr_len);
        if (valread < 8) {
            // Too small packet received
            uint16_t sequence_number;
            memcpy(&sequence_number, buffer + 1, 2);
            sequence_number = ntohs(sequence_number);
            send_error_packet(server_fd, &client_addr, addr_len, sequence_number, 1);
            continue;
        }

        uint8_t message_type = buffer[0];
        uint16_t sequence_number;
        memcpy(&sequence_number, buffer + 1, 2);
        sequence_number = ntohs(sequence_number);
        uint8_t ttl = buffer[3];
        uint32_t payload_length;
        memcpy(&payload_length, buffer + 4, 4);
        payload_length = ntohl(payload_length);
        memcpy(payload, buffer + 8, valread - 8);
        payload[payload_length] = '\0';

        if (payload_length < 100) {
            send_error_packet(server_fd, &client_addr, addr_len, sequence_number, 1);
            continue;
        }
        if (payload_length > MAX_SIZE) {
            // Too large payload length
            send_error_packet(server_fd, &client_addr, addr_len, sequence_number, 3);
            continue;
        }

        if (strlen(payload) != payload_length) {
            // Payload length and payload inconsistent
            send_error_packet(server_fd, &client_addr, addr_len, sequence_number, 2);
            continue;
        }

        if (ttl % 2 != 0) {
            // TTL value is not even
            send_error_packet(server_fd, &client_addr, addr_len, sequence_number, 4);
            continue;
        }

        // Decrement TTL and send the packet back to the client
        ttl--;
        buffer[3] = ttl;
        sendto(server_fd, buffer, valread, 0, (struct sockaddr *)&client_addr, addr_len);
    }

    close(server_fd);
    return 0;
}
