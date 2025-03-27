#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <stdint.h>
#include <sys/time.h>

#define MAX_SIZE 5000

void print_error_type(uint8_t error_code) {
    switch (error_code) {
        case 1:
            printf("Error: TOO SMALL PACKET RECEIVED\n");
            break;
        case 2:
            printf("Error: PAYLOAD LENGTH AND PAYLOAD INCONSISTENT\n");
            break;
        case 3:
            printf("Error: TOO LARGE PAYLOAD LENGTH\n");
            break;
        case 4:
            printf("Error: TTL VALUE IS NOT EVEN\n");
            break;
        default:
            printf("Unknown Error Code: %d\n", error_code);
    }
}

int main(int argc, char *argv[]) {
    if (argc != 6) {
        fprintf(stderr, "Usage: %s <server_ip> <server_port> <ttl> <payload_length> <no_of_packets>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    int sock;
    struct sockaddr_in server_addr;
    uint8_t buffer[8 + MAX_SIZE]; // Packet buffer: 1 byte MT + 2 bytes SN + 1 byte TTL + 4 bytes PL + P bytes PB

    // Create UDP socket
    if ((sock = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        perror("Socket failed");
        exit(EXIT_FAILURE);
    }

    // Configure server address
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = inet_addr(argv[1]);
    server_addr.sin_port = htons(atoi(argv[2]));

    int no_of_packets = atoi(argv[5]);
    uint8_t ttl = (uint8_t)atoi(argv[4]);
    uint32_t payload_length = (uint32_t)atoi(argv[3]);

    if (payload_length > MAX_SIZE) {
        fprintf(stderr, "Error: Payload too large!\n");
        close(sock);
        exit(EXIT_FAILURE);
    }

    uint8_t message_type = 1;
    uint16_t sequence_number = 0;
    uint8_t payload[MAX_SIZE];

    // Fill the payload with 'aaa'
    memset(payload, 'a', payload_length);
    payload[payload_length] = '\0'; // Null-terminate the payload

    for (int i = 0; i < no_of_packets; i++) {
        sequence_number = (sequence_number + 1) % 50;

        // Construct packet
        buffer[0] = message_type;
        uint16_t network_sequence_number = htons(sequence_number);
        memcpy(buffer + 1, &network_sequence_number, 2);
        buffer[3] = ttl;
        uint32_t network_payload_length = htonl(payload_length);
        memcpy(buffer + 4, &network_payload_length, 4);
        memcpy(buffer + 8, payload, payload_length);

        // Record the time before sending the packet
        struct timeval start, end;
        gettimeofday(&start, NULL);

        // Send the packet
        sendto(sock, buffer, 8 + payload_length, 0, (struct sockaddr *)&server_addr, sizeof(server_addr));

        // Receive the response
        socklen_t addr_len = sizeof(server_addr);
        int valread = recvfrom(sock, buffer, sizeof(buffer), 0, (struct sockaddr *)&server_addr, &addr_len);
        if (valread < 0) {
            perror("recvfrom failed");
            return -1;
        }
    
        uint8_t message_type = buffer[0]; // First byte is the error code
    
        // If server sent an error packet, you will receive type 1/2/3/4
        print_error_type(message_type);

        // Record the time after receiving the response
        gettimeofday(&end, NULL);

        // Calculate RTT
        long seconds = end.tv_sec - start.tv_sec;
        long microseconds = end.tv_usec - start.tv_usec;
        double rtt = seconds + microseconds * 1e-6;

        printf("RTT for packet %d: %.6f seconds\n", i + 1, rtt);
    }

    // Close socket
    close(sock);
    return 0;
}
